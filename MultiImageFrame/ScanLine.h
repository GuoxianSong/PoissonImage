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

	std::vector<std::list<EDGE> >	net_;			//�±߱� New Edge Table from top_ to bottom_
	std::list<EDGE>					aet_;			//��߱� Active Edge Table
	std::vector<double>				intersects_;	//��ǰɨ���߽���

	void constructNet();							//�����±߱�
	void deleteOldEDGEs(int);						//ɾ������ȫɨ���ı�
	void transCurrentEDGEs();						//��߱��н������
	void insertNewEDGEs(int);						//�����±�
	static bool edgeCompareByX(EDGE, EDGE);			//������������С�Ƚ�
	void calcIntersects(int);						//���㵱ǰɨ���߽���
	void markInsidePoints(int);						//����ڵ�

public:
	CScanLine(void);
	~CScanLine(void);

	void init(int,int,QPolygon);
	Eigen::MatrixXi insideMatrix();					//�����ڵ����
};

