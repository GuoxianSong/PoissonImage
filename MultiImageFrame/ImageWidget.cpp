#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include <algorithm>
#include <cmath>
#include "ChildWindow.h"
#include "ScanLine.h"
#include "PoissonEditGrad.h"
#include "PoissonEditMixGrad.h"
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow *relatewindow)
{
	image_ = new QImage();
	image_backup_ = new QImage();

	draw_status_ = kNone;
	region_shape_ = kRectangle;
	paste_method_ = kCopy;
	is_choosing_ = false;
	is_pasting_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);
	rect_or_ellipse_ = QRect(point_start_,point_end_);
	polygon_.clear();
	poissonEdit_ = NULL;
	//time_count_ = 0;

	source_window_ = NULL;
}

ImageWidget::~ImageWidget(void)
{
	if (poissonEdit_)
		delete poissonEdit_;
}

int ImageWidget::ImageWidth()
{
	return image_->width();
}

int ImageWidget::ImageHeight()
{
	return image_->height();
}

void ImageWidget::set_draw_status_to_choose()
{
	draw_status_ = kChoose;
	ClearChosen();
	update();
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
}

void ImageWidget::set_region_shape(int shape)
{
	switch (shape)
	{
	case kRectangle:
		region_shape_ = kRectangle;
		break;
	case kEllipse:
		region_shape_ = kEllipse;
		break;
	case kPolygon:
		region_shape_ = kPolygon;
		break;
	default:
		break;
	}
}

void ImageWidget::set_paste_method(int method)
{
	switch (method)
	{
	case kCopy:
		paste_method_ = kCopy;
		break;
	case kGrad:
		paste_method_ = kGrad;
		break;
	case kMixGrad:
		paste_method_ = kMixGrad;
		break;
	default:
		break;
	}
}

QImage* ImageWidget::image()
{
	return image_;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent *paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect( 0, 0, image_->width(), image_->height());
	painter.drawImage(rect, *image_); 

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);
	switch (region_shape_)
	{
	case kRectangle:
		painter.drawRect(point_start_.x(), point_start_.y(), 
			point_end_.x()-point_start_.x(), point_end_.y()-point_start_.y());
		break;
	case kEllipse:
		painter.drawEllipse(point_start_.x(), point_start_.y(), 
			point_end_.x()-point_start_.x(), point_end_.y()-point_start_.y());
		break;
	case kPolygon:
		if (is_choosing_)
		{
			painter.drawPolyline(polygon_);
			painter.drawLine(point_start_,point_end_);
		}
		else
			painter.drawPolygon(polygon_);
		break;
	default:
		break;
	}
	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent *mouseevent)
{
	if (Qt::LeftButton == mouseevent->button())
	{
		switch (draw_status_)
		{
		case kChoose:
			is_choosing_ = true;
			switch (region_shape_)
			{
			case kRectangle:
			case kEllipse:
				point_start_ = point_end_ = mouseevent->pos();
				break;
			case kPolygon:
				if (polygon_.empty())
				{
					point_start_ = point_end_ = mouseevent->pos();
					polygon_<<point_start_;
				}
				else
				{
					point_end_ = mouseevent->pos();
					update();
				}
				break;
			default:
				break;
			}
			break;

		case kPaste:
			{
				is_pasting_ = true;

				// Start point in object image
				int xpos = mouseevent->pos().rx();
				int ypos = mouseevent->pos().ry();

				// Start point in source image
				int xsourcepos = source_window_->imagewidget_->rect_or_ellipse_.left();
				int ysourcepos = source_window_->imagewidget_->rect_or_ellipse_.top();

				// Width and Height of rectangle region
				int w = source_window_->imagewidget_->rect_or_ellipse_.right()
					-source_window_->imagewidget_->rect_or_ellipse_.left()+1;
				int h = source_window_->imagewidget_->rect_or_ellipse_.bottom()
					-source_window_->imagewidget_->rect_or_ellipse_.top()+1;

				// Paste
				if ( (xpos+w<image_->width()) && (ypos+h<image_->height()) )
				{
					image_backup_vec_.push_back(*image_backup_);
					*image_backup_ = *image_;
					// Restore image
					*(image_) = *(image_backup_);

					//计算新图像
					if (kCopy != paste_method_)
						poissoned_img_ = poissonEdit_->GetPoissonedImage(xpos,ypos);

					// Paste
					switch (paste_method_)
					{
					case kCopy:
						for (int i=0; i<w; i++)
							for (int j=0; j<h; j++)
								if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j))
									image_->setPixel(xpos+i, ypos+j, source_window_->imagewidget_->image()->pixel(xsourcepos+i, ysourcepos+j));
						break;
					case kGrad:
					case kMixGrad:
						for (int i=0; i<w; i++)
							for (int j=0; j<h; j++)
								if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j))
									image_->setPixel(xpos+i, ypos+j, poissoned_img_(xsourcepos+i,ysourcepos+j));
						break;
					default:
						break;
					}
				}
			}
			
			update();
			break;

		default:
			break;
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent *mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		// Store point position for rectangle region
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
		}
		break;

	case kPaste:
		// Paste rectangle region to object image
		if (is_pasting_)
		{
			// Start point in object image
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			
			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->rect_or_ellipse_.left();
			int ysourcepos = source_window_->imagewidget_->rect_or_ellipse_.top();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->rect_or_ellipse_.right()
				-source_window_->imagewidget_->rect_or_ellipse_.left()+1;
			int h = source_window_->imagewidget_->rect_or_ellipse_.bottom()
				-source_window_->imagewidget_->rect_or_ellipse_.top()+1;

			// Paste
			if ( (xpos>0) && (ypos>0) && (xpos+w<image_->width()) && (ypos+h<image_->height()) )
			{
				// Restore image 
				*(image_) = *(image_backup_);

				//计算新图像
				if (kCopy != paste_method_)
					poissoned_img_ = poissonEdit_->GetPoissonedImage(xpos,ypos);

				// Paste
				switch (paste_method_)
				{
				case kCopy:
					for (int i=0; i<w; i++)
						for (int j=0; j<h; j++)
							if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j))
								image_->setPixel(xpos+i, ypos+j, source_window_->imagewidget_->image()->pixel(xsourcepos+i, ysourcepos+j));
					break;
				case kGrad:
				case kMixGrad:
					for (int i=0; i<w; i++)
						for (int j=0; j<h; j++)
							if (1 == source_window_->imagewidget_->inside_points_(xsourcepos+i, ysourcepos+j))
								image_->setPixel(xpos+i, ypos+j, poissoned_img_(xsourcepos+i,ysourcepos+j));
					break;
				default:
					break;
				}
			}
		}

	default:
		break;
	}

	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
			QPoint topLeft,bottomRight;
			switch (region_shape_)
			{
			case kRectangle:
			case kEllipse:
				//找出外框矩形
				topLeft.setX(min(point_start_.x(),point_end_.x()));
				topLeft.setY(min(point_start_.y(),point_end_.y()));
				bottomRight.setX(max(point_start_.x(),point_end_.x()));
				bottomRight.setY(max(point_start_.y(),point_end_.y()));
				rect_or_ellipse_ = QRect(topLeft,bottomRight);
				is_choosing_ = false;
				draw_status_ = kNone;
				find_inside_points();
				break;
			case kPolygon:
				polygon_<<point_end_;
				point_start_ = point_end_;
				break;
			default:
				break;
			}
		}

	case kPaste:
		if (is_pasting_)
		{
			is_pasting_ = false;
			draw_status_ = kNone;
		}

	default:
		break;
	}
	
	update();
}

void ImageWidget::mouseDoubleClickEvent(QMouseEvent *mouseevent)
{
	int top=1000000,bottom=-1,left=10000000,right=-1;
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			switch (region_shape_)
			{
			case kPolygon:
				point_start_ = point_end_ = mouseevent->pos();
				polygon_.push_back(point_end_);
				for (int i=0;i<polygon_.size();i++)
				{
					if (polygon_[i].x()<left)
						left = polygon_[i].x();
					if(polygon_[i].x()>right)
						right = polygon_[i].x();
					if(polygon_[i].y()>bottom)
						bottom = polygon_[i].y();
					if(polygon_[i].y()<top)
						top = polygon_[i].y();
				}
				if (top < 0)
					top =0;
				if (bottom >= ImageHeight())
					bottom = ImageHeight()-1;
				if (left < 0)
					left = 0;
				if (right >= ImageWidth())
					right = ImageWidth()-1;
				//找外框矩形
				rect_or_ellipse_ = QRect(QPoint(left,top),QPoint(right,bottom));
				is_choosing_ = false;
				draw_status_ = kNone;
				find_inside_points();
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
	update();
}

void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
		image_->load(filename);
		*(image_backup_) = *(image_);
		inside_points_.resize(image_->width(),image_->height());
		inside_points_ = Eigen::MatrixXi::Zero(image_->width(),image_->height());
	}

//	setFixedSize(image_->width(), image_->height());
//	relate_window_->setWindowFlags(Qt::Dialog);
//	relate_window_->setFixedSize(QSize(image_->width(), image_->height()));
//	relate_window_->setWindowFlags(Qt::SubWindow);
	
	//image_->invertPixels(QImage::InvertRgb);
	//*(image_) = image_->mirrored(true, true);
	//*(image_) = image_->rgbSwapped();
	cout<<"image size: "<<image_->width()<<' '<<image_->height()<<endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}	

	image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i=0; i<image_->width(); i++)
	{
		for (int j=0; j<image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			image_->setPixel(i, j, qRgb(255-qRed(color), 255-qGreen(color), 255-qBlue(color)) );
		}
	}

	// equivalent member function of class QImage
	// image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(image_));
	int width = image_->width();
	int height = image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width-1-i, height-1-j));
				}
			}
		} 
		else
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(i, height-1-j));
				}
			}
		}
		
	}
	else
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width-1-i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(image_) = image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i=0; i<image_->width(); i++)
	{
		for (int j=0; j<image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			int gray_value = (qRed(color)+qGreen(color)+qBlue(color))/3;
			image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value) );
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(image_) = *(image_backup_);
	if (!image_backup_vec_.empty())
	{
		*image_backup_ = image_backup_vec_.back();
		image_backup_vec_.pop_back();
	}
	point_start_ = point_end_ = QPoint(0, 0);
	rect_or_ellipse_ = QRect(point_start_,point_end_);
	polygon_.clear();
	update();
}

void ImageWidget::find_inside_points()
{
	int a,b,c;
	CScanLine scanline;
	switch (region_shape_)
	{
	case kRectangle:
		for (int i=rect_or_ellipse_.left();i<=rect_or_ellipse_.right();i++)
			for (int j=rect_or_ellipse_.top();j<=rect_or_ellipse_.bottom();j++)
				inside_points_(i,j) = 1;
		break;
	case kEllipse:
		a = (rect_or_ellipse_.right()-rect_or_ellipse_.left())/2;
		b = (rect_or_ellipse_.bottom()-rect_or_ellipse_.top())/2;
		c = (int)sqrt(double(b*b-a*a));
		for (int i=rect_or_ellipse_.left();i<=rect_or_ellipse_.right();i++)
			for (int j=rect_or_ellipse_.top();j<=rect_or_ellipse_.bottom();j++)
			{
				int x = i-(rect_or_ellipse_.left()+a);
				int y = j-(rect_or_ellipse_.top()+b);
				if (x*x/double(a*a)+y*y/double(b*b)<=1.0)
					inside_points_(i,j) = 1;
			}
		break;
	case kPolygon:
		//扫描线
		scanline.init(ImageWidth(),ImageHeight(),polygon_);
		inside_points_ = scanline.insideMatrix();
		break;
	default:
		break;
	}
	//找边界
	find_boundary();
}

void ImageWidget::find_boundary()
{
	for (int i=0;i<ImageWidth();i++)
		for (int j=0;j<ImageHeight();j++)
			if (0 == inside_points_(i,j))
			{
				if (i+1<ImageWidth() && 1 == inside_points_(i+1,j))
					inside_points_(i,j) = 2;
				else if (i>0 && 1 == inside_points_(i-1,j))
					inside_points_(i,j) = 2;
				else if (j+1<ImageHeight() && 1 == inside_points_(i,j+1))
					inside_points_(i,j) = 2;
				else if (j>0 && 1 == inside_points_(i,j-1))
					inside_points_(i,j) = 2;
			}
}

void ImageWidget::test_inside_points()			//测试扫描线，内点置白外点置黑。用户接口已屏蔽
{
	for (int i=0; i<image_->width(); i++)
	{
		for (int j=0; j<image_->height(); j++)
		{
			QRgb color;
			if (1 == inside_points_(i,j))
				color = qRgb(255,255,255);
			else
				color = qRgb(0,0,0);
			image_->setPixel(i, j, color);
		}
	}
	update();
}

void ImageWidget::matrix_predecomp()
{
	//删除旧的Poisson对象
	if (poissonEdit_)
		delete poissonEdit_;

	switch (paste_method_)
	{
	case kGrad:
		poissonEdit_ = new CPoissonEditGrad;
		break;
	case kMixGrad:
		poissonEdit_ = new CPoissonEditMixGrad;
		break;
	default:
		break;
	}
	poissonEdit_->init(source_window_->imagewidget_,this);
}

void ImageWidget::ClearChosen()
{
	point_start_ = point_end_ = QPoint(0,0);
	polygon_.clear();
	inside_points_ = Eigen::MatrixXi::Zero(ImageWidth(),ImageHeight());
}