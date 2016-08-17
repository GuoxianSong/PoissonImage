#pragma once
#include <QWidget>
#include <Eigen/Dense>

class ChildWindow;
class CPoissonEdit;
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
class QRect;
class QPolygon;
QT_END_NAMESPACE

enum PasteMethods
{
	kCopy,					//ֱ�Ӹ���
	kGrad,					//Poisson����
	kMixGrad				//POisson͸��
};

enum RegionShape
{
	kRectangle,				//���ƾ���
	kEllipse,				//������Բ
	kPolygon				//���ƶ����
};

enum DrawStatus
{
	kChoose, 
	kPaste, 
	kNone
};

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(ChildWindow *relatewindow);
	~ImageWidget(void);

	int ImageWidth();											// Width of image
	int ImageHeight();											// Height of image
	void set_draw_status_to_choose();
	void set_draw_status_to_paste();
	void set_region_shape(int);
	void set_paste_method(int);
	QImage* image();
	void set_source_window(ChildWindow* childwindow);
	void find_inside_points();									//��ѡ����������ڵ�
	void find_boundary();										//����߽��
	void test_inside_points();
	void matrix_predecomp();									//�����LLTԤ�ֽ�
	void ClearChosen();											//���ѡ������

protected:
	void paintEvent(QPaintEvent *paintevent);
	void mousePressEvent(QMouseEvent *mouseevent);
	void mouseMoveEvent(QMouseEvent *mouseevent);
	void mouseReleaseEvent(QMouseEvent *mouseevent);
	void mouseDoubleClickEvent(QMouseEvent *mouseevent);

public slots:
	// File IO
	void Open(QString filename);								// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal=false, bool vertical=true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin �޸�:�ָ�Ϊ��һ״̬

public:
	QPoint						point_start_;					// Left top point of rectangle region
	QPoint						point_end_;						// Right bottom point of rectangle region
	QRect						rect_or_ellipse_;				//ѡ������ľ������
	QPolygon					polygon_;						//ѡ��Ķ����
	Eigen::MatrixXi				inside_points_;					//�ڵ���Ϊ1�������Ϊ0���߽���Ϊ2
	CPoissonEdit				*poissonEdit_;					//Poisson�������
	Eigen::MatrixXi				poissoned_img_;					//��������ͼ��

private:
	QImage						*image_;						// image 
	QImage						*image_backup_;
	std::vector<QImage>			image_backup_vec_;				//ͼ����ջ
	//int							time_count_;

	// Pointer of child window
	ChildWindow					*source_window_;				// Source child window

	// Signs
	DrawStatus					draw_status_;					// Enum type of draw status
	RegionShape					region_shape_;					//ѡ���������״
	PasteMethods				paste_method_;					//ճ������
	bool						is_choosing_;
	bool						is_pasting_;
};

