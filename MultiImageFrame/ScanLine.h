#pragma once
#include <QWidget>
#include <vector>
#include <list>
#include <Eigen/Dense>

class QPolygon;

typedef struct
{
	int ymax;
	double dx;
	double x;
}EDGE;

class CScanLine
{
	int width_,height_;
	Eigen::MatrixXi inside_points_;
	QPolygon polygon_;
	int top_,bottom_,left_,right_;

	std::vector<std::list<EDGE> >	net_;			//新边表 New Edge Table from top_ to bottom_
	std::list<EDGE>					aet_;			//活动边表 Active Edge Table
	std::vector<double>				intersects_;	//当前扫描线交点

	void constructNet();							//构建新边表
	void deleteOldEDGEs(int);						//删除已完全扫过的边
	void transCurrentEDGEs();						//活动边表中交点更新
	void insertNewEDGEs(int);						//插入新边
	static bool edgeCompareByX(EDGE, EDGE);			//按交点横坐标大小比较
	void calcIntersects(int);						//计算当前扫描线交点
	void markInsidePoints(int);						//标记内点

public:
	CScanLine(void);
	~CScanLine(void);

	void init(int,int,QPolygon);
	Eigen::MatrixXi insideMatrix();					//返回内点矩阵
};

