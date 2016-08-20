#ifndef __EACSMB_VIEW_H__
#define __EACSMB_VIEW_H__


typedef struct ViewInfo {
	Matrix* matWorldView;
	Matrix* matViewProjection;
	
	Vector world_eyePos;
	Vector world_eyeDir;
	Vector world_eyeUp;
	Vector world_eyeRight;
} ViewInfo;






#endif // __EACSMB_VIEW_H__