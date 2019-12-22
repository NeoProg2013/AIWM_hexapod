#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QVariant>

typedef struct {
	float x;
	float y;
	float z;
} point_3d_t;

typedef enum {
	LINK_COXA,
	LINK_FEMUR,
	LINK_TIBIA
} link_id_t;

typedef enum {
	PATH_LINEAR,
	PATH_XZ_ARC_Y_LINEAR,
	PATH_XZ_ARC_Y_SINUS,
	PATH_XZ_ELLIPTICAL_Y_SINUS,
	PATH_YZ_ELLIPTICAL_X_SINUS
} path_type_t;

typedef struct {

	path_type_t path_type;
	point_3d_t  start_point;
	point_3d_t  dest_point;

} path_3d_t;

class Core : public QObject {

	Q_OBJECT

private:

protected:
	void pathCalculatePoint(const path_3d_t* info, uint32_t current_iteration, point_3d_t* point);
	bool kinematic_calculate_angles(point_3d_t& position);

signals:
	void pointCalculated(QVariant x, QVariant y, QVariant z, QVariant isSuccess);

public:
	explicit Core(QObject *parent = nullptr);
	Q_INVOKABLE void calculateRequest(QVariant x0, QVariant y0, QVariant z0,
									  QVariant x1, QVariant y1, QVariant z1,
									  QVariant algType);
};

#endif // CORE_H
