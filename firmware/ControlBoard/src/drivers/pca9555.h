/// ***************************************************************************
/// @file    PCA9555.h
/// @author  NeoProg
/// @brief   PCA9555 driver
/// ***************************************************************************
#ifndef _PCA9555_H_
#define _PCA9555_H_

extern bool pca9555_init(void);
extern bool pca9555_is_input_changed(void);
extern bool pca9555_read(uint8_t* data);
extern bool pca9555_write(uint16_t data);

#endif // _PCA9555_H_
