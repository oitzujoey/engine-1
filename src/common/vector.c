
#include "vector.h"
#include <math.h>
#include "common.h"

void vec3_copy(vec3_t *destination, vec3_t *source) {
	(*destination)[0] = (*source)[0];
	(*destination)[1] = (*source)[1];
	(*destination)[2] = (*source)[2];
}

/*
a: First input to dot product.
b: Second input to dot product.
returns: Dot product.
*/
void vec3_dotProduct(vec_t *result, vec3_t *a, vec3_t *b) {
	// Ewww!
	*result = (*a)[0]*(*b)[0] + (*a)[1]*(*b)[1] + (*a)[2]*(*b)[2];
}

void vec3_crossProduct(vec3_t *result, vec3_t *a, vec3_t *b) {
	// Ewww!
	(*result)[0] = (*a)[1]*(*b)[2] - (*a)[2]*(*b)[1];
	(*result)[1] = (*a)[2]*(*b)[0] - (*a)[0]*(*b)[2];
	(*result)[2] = (*a)[0]*(*b)[1] - (*a)[1]*(*b)[0];
}

void vec3_subtract(vec3_t *result, vec3_t *a, vec3_t *b) {
	(*result)[0] = (*a)[0] - (*b)[0];
	(*result)[1] = (*a)[1] - (*b)[1];
	(*result)[2] = (*a)[2] - (*b)[2];
}

int vec3_normalize(vec3_t *v) {
	int error = ERR_OK;

	vec_t magnitude = sqrt((*v)[0]*(*v)[0] + (*v)[1]*(*v)[1] + (*v)[2]*(*v)[2]);
	if (magnitude == 0.0) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	(*v)[0] /= magnitude;
	(*v)[1] /= magnitude;
	(*v)[2] /= magnitude;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void quat_copy(quat_t *out, quat_t *in) {
	*out = *in;
}

void quat_hamilton(quat_t *out, quat_t *q0, quat_t *q1) {
	quat_t temp;
	temp.s    = (q0->s * q1->s   ) - (q0->v[0] * q1->v[0]) - (q0->v[1] * q1->v[1]) - (q0->v[2] * q1->v[2]);
	temp.v[0] = (q0->s * q1->v[0]) + (q0->v[0] * q1->s   ) + (q0->v[1] * q1->v[2]) - (q0->v[2] * q1->v[1]);
	temp.v[1] = (q0->s * q1->v[1]) - (q0->v[0] * q1->v[2]) + (q0->v[1] * q1->s   ) + (q0->v[2] * q1->v[0]);
	temp.v[2] = (q0->s * q1->v[2]) + (q0->v[0] * q1->v[1]) - (q0->v[1] * q1->v[0]) + (q0->v[2] * q1->s   );
	*out = temp;
}

void quat_conjugate(quat_t *q) {
	q->v[0] = -q->v[0];
	q->v[1] = -q->v[1];
	q->v[2] = -q->v[2];
}

int quat_normalize(quat_t *q) {
	int error = ERR_OK;

	vec_t magnitude = sqrt(q->s * q->s + q->v[0] * q->v[0] + q->v[1] * q->v[1] + q->v[2] * q->v[2]);
	if (magnitude == 0.0) {
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	q->s    = q->s    / magnitude;
	q->v[0] = q->v[0] / magnitude;
	q->v[1] = q->v[1] / magnitude;
	q->v[2] = q->v[2] / magnitude;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void quat_inverse(quat_t *q) {
	vec_t norm2 = q->s * q->s + q->v[0] * q->v[0] + q->v[1] * q->v[1] + q->v[2] * q->v[2];
	q->s    = q->s    / norm2;
	q->v[0] = q->v[0] / norm2;
	q->v[1] = q->v[1] / norm2;
	q->v[2] = q->v[2] / norm2;
}

// Assume q is a unit quaternion.
void vec3_rotate(vec3_t *v, quat_t *q) {
	quat_t vector;
	quat_t qInverse;
	
	vector.s = 0;
	vec3_copy(&vector.v, v);
	
	quat_copy(&qInverse, q);
	quat_unitInverse(&qInverse);
	
	quat_hamilton(&vector, q, &vector);
	quat_hamilton(&vector, &vector, &qInverse);
	
	vec3_copy(v, &vector.v);
}
