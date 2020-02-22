#include "core.h"
#include <math.h>
#include <QDebug>
#define RAD_TO_DEG(rad)                ((rad) * 180.0f / M_PI)
#define DEG_TO_RAD(deg)                ((deg) * M_PI / 180.0f)
#define TOTAL_ITERATION_COUNT		   (36)





Core::Core(QObject* parent) : QObject(parent) {

}

void Core::calculateRequest(QVariant x0, QVariant y0, QVariant z0,
							QVariant x1, QVariant y1, QVariant z1,
							QVariant algType) {

	path_3d_t path;
	path.path_type = static_cast<path_type_t>(algType.toInt());
	path.start_point.x = x0.toInt();
	path.start_point.y = y0.toInt();
	path.start_point.z = z0.toInt();

	path.dest_point.x = x1.toInt();
	path.dest_point.y = y1.toInt();
	path.dest_point.z = z1.toInt();

	for (uint32_t i = 0; i <= TOTAL_ITERATION_COUNT; ++i) {

		point_3d_t point;
		pathCalculatePoint(&path, i, &point);

		bool result = kinematic_calculate_angles(point);

		emit pointCalculated(point.x, point.y, point.z, result);
	}
}

void Core::pathCalculatePoint(const path_3d_t* info, uint32_t current_iteration, point_3d_t* point) {

	float t_max = RAD_TO_DEG(M_PI); // [0; Pi]
	float t = current_iteration * (t_max / TOTAL_ITERATION_COUNT); // iter_index * dt

	float x0 = info->start_point.x;
	float y0 = info->start_point.y;
	float z0 = info->start_point.z;
	float x1 = info->dest_point.x;
	float y1 = info->dest_point.y;
	float z1 = info->dest_point.z;


	if (info->path_type == PATH_LINEAR) {
		point->x = t * (x1 - x0) / t_max + x0;
		point->y = t * (y1 - y0) / t_max + y0;
		point->z = t * (z1 - z0) / t_max + z0;
	}

	if (info->path_type == PATH_XZ_ARC_Y_LINEAR) {

		float R = sqrt(x0 * x0 + z0 * z0);
		float atan0 = RAD_TO_DEG(atan2(x0, z0));
		float atan1 = RAD_TO_DEG(atan2(x1, z1));

		float t_mapped_rad = DEG_TO_RAD(t * (atan1 - atan0) / t_max + atan0);

		point->x = R * sin(t_mapped_rad); // circle Y
		point->y = t * (info->dest_point.y - info->start_point.y) / t_max + info->start_point.y;
		point->z = R * cos(t_mapped_rad); // circle X

		qDebug() << point->x << " " << point->y << " " << point->z;
	}

	if (info->path_type == PATH_XZ_ARC_Y_SINUS) {

		float R = sqrt(x0 * x0 + z0 * z0);
		float atan0 = RAD_TO_DEG(atan2(x0, z0));
		float atan1 = RAD_TO_DEG(atan2(x1, z1));

		float t_mapped_rad = DEG_TO_RAD(t * (atan1 - atan0) / t_max + atan0);

		point->x = R * sin(t_mapped_rad); // circle Y
		point->y = (y1 - y0) * sin(DEG_TO_RAD(t)) + y0;
		point->z = R * cos(t_mapped_rad); // circle X

		qDebug() << point->x << " " << point->y << " " << point->z;
	}

	if (info->path_type == PATH_XZ_ELLIPTICAL_Y_SINUS) {

		float a = (z1 - z0) / 2.0f;
		float b = (x1 - x0);
		float c = (y1 - y0);

		point->x = b * sin(DEG_TO_RAD(t_max - t)) + x0; // circle Y
		point->y = c * sin(DEG_TO_RAD(t)) + y0;
		point->z = a * cos(DEG_TO_RAD(t_max - t)) + z0 + a;

		qDebug() << point->x << " " << point->y << " " << point->z;
	}

	if (info->path_type == PATH_YZ_ELLIPTICAL_X_SINUS) {

		/*float a = (z1 - z0) / 2.0f;
		float b = (y1 - y0);

		point->x = t * (info->dest_point.x - info->start_point.x) / t_max + info->start_point.x;
		point->y = b * sin(DEG_TO_RAD(t_max - t)) + y0;
		point->z = a * cos(DEG_TO_RAD(t_max - t)) + z0 + a;

		qDebug() << point->x << " " << point->y << " " << point->z;*/

		float a = (x1 - x0) / 2.0f;
		float b = (z1 - z0);
		float c = (y1 - y0);

		point->x = a * cos(DEG_TO_RAD(t_max - t)) + x0 + a; // circle X
		point->y = c * sin(DEG_TO_RAD(t)) + y0;
		point->z = b * sin(DEG_TO_RAD(t_max - t)) + z0; // circle Y

		qDebug() << point->x << " " << point->y << " " << point->z;
	}
}

bool Core::kinematic_calculate_angles(point_3d_t& position) {

	int32_t coxa_zero_rotate_deg = 45;
	int32_t femur_zero_rotate_deg = 125;
	int32_t tibia_zero_rotate_deg = 45;
	uint32_t coxa_length = 53;
	uint32_t femur_length = 76;
	uint32_t tibia_length = 137;

	float x = position.x;
	float y = position.y;
	float z = position.z;


	// Move to (X*, Y*, Z*) coordinate system - rotate
	float coxa_zero_rotate_rad = DEG_TO_RAD(coxa_zero_rotate_deg);
	float x1 = x * cos(coxa_zero_rotate_rad) + z * sin(coxa_zero_rotate_rad);
	float y1 = y;
	float z1 = -x * sin(coxa_zero_rotate_rad) + z * cos(coxa_zero_rotate_rad);


	//
	// Calculate COXA angle
	//
	float coxa_angle_rad = atan2(z1, x1);
	float coxa_angle = RAD_TO_DEG(coxa_angle_rad);


	//
	// Prepare for calculation FEMUR and TIBIA angles
	//
	// Move to (X*, Y*) coordinate system (rotate on axis Y)
	x1 = x1 * cos(coxa_angle_rad) + z1 * sin(coxa_angle_rad);

	// Move to (X**, Y**) coordinate system (remove coxa from calculations)
	x1 = x1 - coxa_length;

	// Calculate angle between axis X and destination point
	float fi = atan2(y1, x1);

	// Calculate distance to destination point
	float d = sqrt(x1 * x1 + y1 * y1);
	if (d > femur_length + tibia_length) {
		return false; // Point not attainable
	}


	//
	// Calculate triangle angles
	//
	float a = tibia_length;
	float b = femur_length;
	float c = d;

	float alpha = acos( (b * b + c * c - a * a) / (2 * b * c) );
	float gamma = acos( (a * a + b * b - c * c) / (2 * a * b) );


	//
	// Calculate FEMUR and TIBIA angle
	//
	float femur_angle = femur_zero_rotate_deg - RAD_TO_DEG(alpha) - RAD_TO_DEG(fi);
	float tibia_angle = RAD_TO_DEG(gamma) - tibia_zero_rotate_deg;

	if (femur_angle < 0 || tibia_angle < 0) {
		return false;
	}


	return true;
}
