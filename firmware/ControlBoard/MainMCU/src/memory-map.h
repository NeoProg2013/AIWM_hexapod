/// ***************************************************************************
/// @file    mamory-map.c
/// @author  NeoProg
/// @brief   Configurator memory map
/// ***************************************************************************
#ifndef _MEMORY_MAP_H_
#define _MEMORY_MAP_H_

#define MM_VERSION                                          (0x0001)



#define MM_VERSION_EE_ADDRESS                               (0x0000)    ///< U16 Memory map version
#define MM_PAGE_CHECKSUM_OFFSET                             (0xFE)      ///< U16 Page checksum
#define MM_PAGE_CHECKSUM_SIZE                               (2)

//
// PAGE 0
//
#define MM_VBAT_OFFSET_EE_ADDRESS                           (0x0010)    ///< U16 VBAT voltage offset


//
// PAGE 1
//
#define MM_LIMB_CONFIG_BASE_EE_ADDRESS                      (0x0100)
#define MM_LIMB_COXA_LENGTH_OFFSET                          (0x00)
#define MM_LIMB_FEMUR_LENGTH_OFFSET                         (0x02)
#define MM_LIMB_TIBIA_LENGTH_OFFSET                         (0x04)
#define MM_LIMB_COXA_ZERO_ROTATE_OFFSET                     (0x10)
#define MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET                    (0x1C)
#define MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET                    (0x1E)
#define MM_LIMB_PROTECTION_COXA_MIN_ANGLE_OFFSET            (0x50)      ///< S16 Min coxa angle (protection)
#define MM_LIMB_PROTECTION_COXA_MAX_ANGLE_OFFSET            (0x52)      ///< S16 Max coxa angle (protection)
#define MM_LIMB_PROTECTION_FEMUR_MIN_ANGLE_OFFSET           (0x54)      ///< S16 Min femur angle (protection)
#define MM_LIMB_PROTECTION_FEMUR_MAX_ANGLE_OFFSET           (0x56)      ///< S16 Max femur angle (protection)
#define MM_LIMB_PROTECTION_TIBIA_MIN_ANGLE_OFFSET           (0x58)      ///< S16 Min tibia angle (protection)
#define MM_LIMB_PROTECTION_TIBIA_MAX_ANGLE_OFFSET           (0x5A)      ///< S16 Max tibia angle (protection)


//
// PAGE 2
//
#define MM_SERVO_CONFIG_BLOCK_BASE_EE_ADDRESS               (0x0200)
#define MM_SERVO_CONFIG_BLOCK_SIZE                          (8)

#define MM_SERVO_CONFIG_OFFSET                              (0)          ///< U8  Servo configuration
#define     MM_SERVO_CONFIG_REVERSE_DIRECTION_MASK          (0x01)
#define     MM_SERVO_CONFIG_SERVO_TYPE_MASK                 (0xF0)
#define MM_SERVO_ZERO_TRIM_OFFSET                           (2)          ///< S16 Servo zero trim


#endif // _MEMORY_MAP_H_
