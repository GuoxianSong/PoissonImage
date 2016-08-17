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
	kCopy,					//直接复制
	kGrad,					//Poisson处理
	kMixGrad				//POisson透明
};

enum RegionShape
{
	kRectangle,				//绘制矩形
	kEllipse,				//绘制椭圆
	kPolygon				//绘制多边形
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
	void find_inside_points();									//对选定区域计算内点
	void find_boundary();										//计算边界点
	void test_inside_points();
	void matrix_predecomp();									//矩阵的LLT预分解
	void ClearChosen();											//清空选择区域

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
	void Restore();												// Restore image to origin 修改:恢复为上一状态

public:
	QPoint						point_start_;					// Left top point of rectangle region
	QPoint						point_end_;						// Right bottom point of rectangle region
	QRect						rect_or_ellipse_;				//选择区域的矩阵外框
	QPolygon					polygon_;						//选择的多边形
	Eigen::MatrixXi				inside_points_;					//内点标记为1，外点标记为0，边界标记为2
	CPoissonEdit				*poissonEdit_;					//Poisson处理对象
	Eigen::MatrixXi				poissoned_img_;					//处理过后的图像

private:
	QImage						*image_;						// image 
	QImage						*image_backup_;
	std::vector<QImage>			image_backup_vec_;				//图像撤销栈
	//int							time_count_;

	// Pointer of child window
	ChildWindow					*source_window_;				// Source child window

	// Signs
	DrawStatus					draw_status_;					// Enum type of draw status
	RegionShape					region_shape_;					//选择区域的形状
	PasteMethods				paste_method_;					//粘贴方法
	bool						is_choosing_;
	bool						is_pasting_;
};

