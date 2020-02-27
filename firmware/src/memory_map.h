//  ***************************************************************************
/// @file    mamory_map.c
/// @author  NeoProg
/// @brief   onfigurator memory map
//  ***************************************************************************
#ifndef _MEMORY_MAP_H_
#define _MEMORY_MAP_H_

#define MM_PAGE_CHECKSUM_OFFSET                             (0xFC)      ///< U32 Page checksum

//
// Configuration map for limbs
//
#define MM_LIMB_CONFIG_BASE_EE_ADDRESS                      (0x0100)
#define MM_LIMB_COXA_LENGTH_OFFSET                          (0x00)
#define MM_LIMB_FEMUR_LENGTH_OFFSET                         (0x02)
#define MM_LIMB_TIBIA_LENGTH_OFFSET                         (0x04)
#define MM_LIMB_COXA_0_3_ZERO_ROTATE_OFFSET                 (0x06)
#define MM_LIMB_COXA_1_4_ZERO_ROTATE_OFFSET                 (0x08)
#define MM_LIMB_COXA_2_5_ZERO_ROTATE_OFFSET                 (0x0A)
#define MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET                    (0x0C)
#define MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET                    (0x0E)

//
// Configuration map for servos
//
#define MM_SERVO_CONFIG_BLOCK_BASE_EE_ADDRESS               (0x0200)
#define MM_SERVO_CONFIG_BLOCK_SIZE                          (8)

#define MM_SERVO_CONFIG_OFFSET                              (0)          ///< U8  Servo configuration
#define     MM_SERVO_CONFIG_REVERSE_DIRECTION_MASK          (0x01)
#define     MM_SERVO_CONFIG_SERVO_TYPE_MASK                 (0xF0)
#define MM_SERVO_PWM_CHANNEL_OFFSET                         (1)          ///< U8  PWM channel
#define MM_SERVO_LOGIC_ZERO_OFFSET                          (2)          ///< U16 Servo logic angle
#define MM_SERVO_PROTECTION_MIN_PHYSIC_ANGLE_OFFSET         (4)          ///< U16 Servo min physic angle (protection)
#define MM_SERVO_PROTECTION_MAX_PHYSIC_ANGLE_OFFSET         (6)          ///< U16 Servo max physic angle (protection)


#endif // _MEMORY_MAP_H_

