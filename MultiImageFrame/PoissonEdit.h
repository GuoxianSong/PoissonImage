#pragma once
#include <QWidget>
#include <vector>
#include "ImageWidget.h"
#include <Eigen/Core>
#include <Eigen/SparseCholesky>

class QPoint;
class QRect;
class ImageWidget;

class CPoissonEdit
{
protected:
	ImageWidget		*source_imagewidget_;				//ԭͼ
	ImageWidget		*imagewidget_;						//��ͼ
	QRect			*source_rect_region_;				//ԭͼ��ѡ������ľ������
	Eigen::MatrixXi *source_inside_points_;				//ԭͼ�ڵ��Ǿ���
	Eigen::MatrixXi poissoned_rgb_;						//���ɵ���ͼ��

	int inside_points_count_;							//ԭͼ�ڵ���
	std::vector<QPoint> index_to_coor_;					//��ŵ�����
	Eigen::MatrixXi coor_to_index_;						//���굽���

	Eigen::SparseMatrix<double> A_;						//����ϵ����
	Eigen::MatrixX3d b_;								//�����Ҷ���
	Eigen::SimplicialLLT<Eigen::SparseMatrix<double> > LLT_;

	int source_width();
	int source_height();
	void buildAMat();									//����ϵ����
	void buildbMat(int, int);							//�����Ҷ���
	virtual int v(QPoint, QPoint, int, QPoint) = 0;		//���Ÿ����๹����ͬ���Ҷ���

public:
	CPoissonEdit(void);
	virtual ~CPoissonEdit(void);

	void init(ImageWidget *, ImageWidget *);
	Eigen::MatrixXi GetPoissonedImage(int, int);		//��Ⲣ������ͼ��
};

