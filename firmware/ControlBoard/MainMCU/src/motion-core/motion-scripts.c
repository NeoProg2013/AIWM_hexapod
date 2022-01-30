/// ***************************************************************************
/// @file    motion-scripts.h
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "motion-scripts.h"

static void common_init(motion_t* motion);

static void up_exec(motion_t* motion);
static void down_exec(motion_t* motion);
static void x_rotate_exec(motion_t* motion);
static void z_rotate_exec(motion_t* motion);
static void xy_rotate_init(motion_t* motion);
static void xy_rotate_exec(motion_t* motion);
static void up_down_exec(motion_t* motion);
static void z_push_pull_exec(motion_t* motion);
static void x_sway_exec(motion_t* motion);
static void square_exec(motion_t* motion);


motion_script_t motion_scripts[MOTION_SCRIPTS_COUNT] = {
    { .init = NULL,           .exec = up_exec          },
    { .init = NULL,           .exec = down_exec        },
    { .init = common_init,    .exec = x_rotate_exec    },
    { .init = common_init,    .exec = z_rotate_exec    },
    { .init = xy_rotate_init, .exec = xy_rotate_exec   },
    { .init = common_init,    .exec = up_down_exec     },
    { .init = common_init,    .exec = z_push_pull_exec },
    { .init = common_init,    .exec = x_sway_exec      },
    { .init = common_init,    .exec = square_exec      }
};



static void common_init(motion_t* motion) {
    motion->surface_rotate.x = 0;
    motion->surface_rotate.y = 0;
    motion->surface_rotate.z = 0;
    motion->surface_point.x = 0;
    motion->surface_point.y = -85;
    motion->surface_point.z = 0;
    motion->cfg.speed = 0;
}
static void up_exec(motion_t* motion) {
    common_init(motion);
    motion->surface_point.y = -85;
    motion->cfg.speed = 60;
}
static void down_exec(motion_t* motion) {
    common_init(motion);
    motion->surface_point.y = -15;
}
static void x_rotate_exec(motion_t* motion) {
    static bool direction = false;
    direction = !direction;

    if (!direction) {
        motion->surface_rotate.x = 15;
    } else {
        motion->surface_rotate.x = -15;
    }
    motion->cfg.speed = 0;
}
static void z_rotate_exec(motion_t* motion) {
    static bool direction = false;
    direction = !direction;

    if (!direction) {
        motion->surface_rotate.z = 15;
    } else {
        motion->surface_rotate.z = -15;
    }
    motion->cfg.speed = 0;
}
static void xy_rotate_init(motion_t* motion) {
    common_init(motion);
    motion->surface_rotate.x = 15;
}
static void xy_rotate_exec(motion_t* motion) {
    motion->surface_rotate.y = 361;
    motion->cfg.speed = 60;
}
static void up_down_exec(motion_t* motion) {
    static bool direction = false;
    direction = !direction;

    if (!direction) {
        motion->surface_point.y = -85;
    } else {
        motion->surface_point.y = -115;
    }
    motion->cfg.speed = 0;
}
static void z_push_pull_exec(motion_t* motion) {
    static bool direction = false;
    direction = !direction;

    if (!direction) {
        motion->surface_point.z = 50;
    } else {
        motion->surface_point.z = -50;
    }
}
static void x_sway_exec(motion_t* motion) {
    static bool direction = false;
    direction = !direction;

    if (direction) {
        motion->surface_rotate.x = -20;
        motion->surface_point.z = 50;
    } else {
        motion->surface_rotate.x = 20;
        motion->surface_point.z = -50;
    }
    motion->cfg.speed = 0;
}

static void square_exec(motion_t* motion) {
    static uint8_t loop = 0;

    switch (loop++) {
    case 0:
        motion->surface_rotate.x = 10;
        motion->surface_rotate.z = 10;
        break;
    case 1:
        motion->surface_rotate.x = -10;
        motion->surface_rotate.z = -10;
        break;
    case 2:
        motion->surface_rotate.x = -10;
        motion->surface_rotate.z = 10;
        break;
    case 3:
        motion->surface_rotate.x = 10;
        motion->surface_rotate.z = -10;
        loop = 0;
        break;
    }
}
