/*
 *  Copyright (C) 2004, 2005, 2006, 2009, 2008, 2009, 2010, 2011 Savoir-Faire Linux Inc.
 *  Author: Tristan Matthews <tristan.matthews@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Additional permission under GNU GPL version 3 section 7:
 *
 *  If you modify this program, or any covered work, by linking or
 *  combining it with the OpenSSL project's OpenSSL library (or a
 *  modified version of that library), containing parts covered by the
 *  terms of the OpenSSL or SSLeay licenses, Savoir-Faire Linux Inc.
 *  grants you additional permission to convey the resulting work.
 *  Corresponding Source for a non-source form of such a combination
 *  shall include the source code for the parts of OpenSSL used as well
 *  as that of the covered work.
 */

#include "video_preview.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h> /* for msg queue */
#include <sys/ipc.h>
#include <sys/sem.h> /* semaphore functions and structs. */
#include <sys/shm.h>

#include <clutter/clutter.h>

#define VIDEO_PREVIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
            VIDEO_PREVIEW_TYPE, VideoPreviewPrivate))

/* This macro will implement the video_preview_get_type function
   and define a parent class pointer accessible from the whole .c file */
G_DEFINE_TYPE (VideoPreview, video_preview, G_TYPE_OBJECT);

static void video_preview_finalize (GObject *gobject);

/* Our private member structure */
struct _VideoPreviewPrivate {
    guint width;
    guint height;
    gchar *shm_buffer;
    gint sem_set_id;
    ClutterActor *texture;
};

typedef struct _FrameInfo {
    unsigned size;
    unsigned width;
    unsigned height;
} FrameInfo;

#if _SEM_SEMUN_UNDEFINED
union semun
{
    int val;				    /* value for SETVAL */
    struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
    unsigned short int *array;	/* array for GETALL & SETALL */
    struct seminfo *__buf;		/* buffer for IPC_INFO */
};
#endif

#define TEMPFILE "/tmp/frame.txt"

static FrameInfo
getFrameInfo()
{
    FrameInfo info;
    FILE *tmp = fopen(TEMPFILE, "r");
    if (fscanf(tmp, "%u\n%u\n%u\n", &info.size, &info.width, &info.height) <= 0)
        g_print("Error: Could not read %s\n", TEMPFILE);
    printf("Size is %u\n", info.size);
    printf("Width is %u\n", info.width);
    printf("Height is %u\n", info.height);
    return info;
}

/* join and/or create a shared memory segment */
    static int
getShm(unsigned numBytes)
{
    key_t key;
    int shm_id;
    /* connect to a segment with 600 permissions
       (r--r--r--) */
    key = ftok("/tmp", 'c');
    shm_id = shmget(key, numBytes, 0644);

    return shm_id;
}

/* attach a shared memory segment */
    static char *
attachShm(int shm_id)
{
    char *data = NULL;

    /* attach to the segment and get a pointer to it */
    data = shmat(shm_id, (void *)0, 0);
    if (data == (char *)(-1)) {
        perror("shmat");
        data = NULL;
    }

    return data;
}

    static void
detachShm(char *data)
{
    /* detach from the segment: */
    if (shmdt(data) == -1) {
        perror("shmdt");
    }
}

static int
get_sem_set()
{
    /* this variable will contain the semaphore set. */
    int sem_set_id;
    key_t key = ftok("/tmp", 'b');

    /* semaphore value, for semctl().                */
    union semun sem_val;

    /* first we get a semaphore set with a single semaphore, */
    /* whose counter is initialized to '0'.                     */
    sem_set_id = semget(key, 1, 0600);
    if (sem_set_id == -1) {
        perror("semget");
        exit(1);
    }
    sem_val.val = 0;
    semctl(sem_set_id, 0, SETVAL, sem_val);
    return sem_set_id;
}


    static void
video_preview_class_init (VideoPreviewClass *klass) 
{
    GObjectClass *gobject_class;                                                  
    gobject_class = G_OBJECT_CLASS (klass); 

    g_type_class_add_private (klass, sizeof (VideoPreviewPrivate));
    gobject_class->finalize = video_preview_finalize;       
    /* Initialize Clutter */
    if (clutter_init (NULL, NULL) != CLUTTER_INIT_SUCCESS)
        g_print("Error: could not initialize clutter!\n");
}

static void
video_preview_init (VideoPreview *self)
{
    VideoPreviewPrivate *priv;

    self->priv = priv = VIDEO_PREVIEW_GET_PRIVATE (self);

    /* update the object state depending on constructor properties */
    FrameInfo info = getFrameInfo();
    priv->width = info.width;
    priv->height = info.height;
    int shm_id = getShm(info.size);
    priv->shm_buffer = attachShm(shm_id);
    priv->sem_set_id = get_sem_set();
    priv->texture = NULL;
}

static void
video_preview_finalize (GObject *obj)
{
    VideoPreview *self = VIDEO_PREVIEW (obj);

    if (!g_idle_remove_by_data((void*)self))
        g_print("Could not remove update texture callback\n");

    /* finalize might be called multiple times, so we must guard against
     * calling g_object_unref() on an invalid GObject.
     */
    if (self->priv->shm_buffer)
    {
        detachShm(self->priv->shm_buffer);
        self->priv->shm_buffer = NULL;
    }

    /* Chain up to the parent class */
    G_OBJECT_CLASS (video_preview_parent_class)->finalize (obj);
}

/*
 * function: sem_wait. wait for frame from other process
 * input:    semaphore set ID.
 * output:   none.
 */
static void
sem_wait(int sem_set_id)
{
    /* structure for semaphore operations.   */
    struct sembuf sem_op;

    /* wait on the semaphore, unless it's value is non-negative. */
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}

/* round integer value up to next multiple of 4 */
static int
round_up_4(int value)
{
    return (value + 3) &~ 3;
}

static void
readFrameFromShm(int width, int height, char *data, int sem_set_id,
        ClutterActor *texture)
{
    sem_wait(sem_set_id);
    clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE(texture),
            (void*)data,
            FALSE,
            width,
            height,
            round_up_4(3 * width),
            3,
            0,
            NULL);
}

static gboolean
updateTexture(gpointer data)
{
    VideoPreview *preview = (VideoPreview *) data;
    VideoPreviewPrivate *priv = VIDEO_PREVIEW_GET_PRIVATE(preview);
    readFrameFromShm(priv->width, priv->height, priv->shm_buffer,
            priv->sem_set_id, priv->texture);
    return TRUE;
}

/**                                                                             
 * video_preview_new:                                                         
 *                                                                              
 * Create a new #VideoPreview instance.                                        
 */                                                                             
VideoPreview *                                                                 
video_preview_new (void)
{
    VideoPreview *result;

    result = g_object_new (VIDEO_PREVIEW_TYPE, NULL);
    return result;
}

static gint
on_stage_delete(ClutterStage *stage, ClutterEvent *event, gpointer data)
{
    (void) event;
    clutter_actor_destroy(CLUTTER_ACTOR(stage));
    /* if (!g_idle_remove_by_data((void*)data))
       g_print("Could not remove update texture callback\n"); */
    /* unref self, FIXME: this signal should be forwarded to the owner */
    g_object_unref(G_OBJECT(data));
    return TRUE; /* don't call the default delete-event handler */
}


int
video_preview_run(VideoPreview *preview)
{
    VideoPreviewPrivate * priv = VIDEO_PREVIEW_GET_PRIVATE(preview);

    ClutterActor *stage;

    /* Get the default stage */
    stage = clutter_stage_get_default ();
    g_signal_connect (stage, "delete-event", G_CALLBACK (on_stage_delete), preview);
    clutter_actor_set_size(stage,
            priv->width,
            priv->height);

    priv->texture = clutter_texture_new();

    clutter_stage_set_title(CLUTTER_STAGE (stage), "Client");
    /* Add ClutterTexture to the stage */
    clutter_container_add(CLUTTER_CONTAINER (stage), priv->texture, NULL);

    /* frames are read and saved here */
    g_idle_add(updateTexture, preview);

    clutter_actor_show_all(stage);

    return 0;
}