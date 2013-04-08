// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <sstream>
#include <list>
#include "glfont.h"
#include "float.h"
#include "math.h"
#include "GLFunctions.h"
#include <atltypes.h>

class IDeletable
{
public:
	virtual ~IDeletable()
	{
	}
};
class IGameObject: public IDeletable
{
public:
	//попытка отрисовать обьект, в случае удачи возврат true
	virtual bool draw() const =0;
	virtual bool test(const double& dX, const double& dY, DWORD dwMiliseconds) const =0;
	virtual double getSpeed() const =0;
};

class CCircleGameObject: public IGameObject
{
public:
	//получает на вход размеры игрового поля в миллиметрах
	CCircleGameObject(const double& dX, const double& dY)
	{
		//выбрасываем случайный радиус в максимально возможных пределах
		//чтобы всё ещё не противоречить условиям тестового задания
		
		//22% от максимума
		m_dRadius=min(dX, dY)*0.5*rand()/RAND_MAX;
		
		//пытаемся выбросить случайное число в диапазоне 
		//[-0.5*dX+m_dRadius, 0.5*dX-m_dRadius]
		//чтобы обьект влез по горизонтали
		m_dStartX=-0.5*dX+m_dRadius+(dX-2*m_dRadius)*rand()/RAND_MAX;
		
		//помешаем обьект максимально высоко с условием его необрезания
		m_dStartY=0.5*dY-m_dRadius;
		

		m_colour.setColor(RGB(255.0*rand()/RAND_MAX, 255.0*rand()/RAND_MAX, 255.0*rand()/RAND_MAX));

		//время рождения обьекта чтобы можно было
		//вычислять его положение исходя из текущего времени на момент вычисления
		//собственно этого положения
		m_dwMiliseconds=::GetTickCount();
	}
protected:
	virtual bool draw() const
	{
		DWORD dwMiliseconds=::GetTickCount();
		double dShiftY=(dwMiliseconds-m_dwMiliseconds)*getSpeed();
		
		if(dShiftY<2*m_dStartY)
		{
			//рисуем диск радиусом  m_dRadius [mm]
			//с координатами центра m_dStartX [mm], m_dStartY-dShiftY [mm]

			glColor3(m_colour);

			glPushMatrix();
			glTranslated(m_dStartX, m_dStartY-dShiftY, 0.0);

			double pi_mul_2 = 2*acos(-1.0);
			double step = 1.0/m_dRadius;

			glBegin(GL_TRIANGLE_FAN);
			for(double a = 0; a < pi_mul_2; a += step) {
				glVertex2d(m_dRadius * cos(a), m_dRadius * sin(a));
			}
			glEnd();
			glPopMatrix();
			return true;
		}
		else
		{
			//обьект отрисовать неудалось, так как он режется краями игрового поля
			//видимо теперь ему пора помирать и исчезать
			return false;
		}
	}
	virtual bool test(const double& dX, const double& dY, DWORD dwMiliseconds) const
	{
		double dShiftY=(dwMiliseconds-m_dwMiliseconds)*getSpeed();

		//тестируем диск радиусом  m_dRadius [mm]
		//с координатами центра m_dStartX [mm], m_dStartY-dShiftY [mm]

		return pow(m_dStartX-dX, 2)+pow(m_dStartY-dShiftY-dY, 2)<pow(m_dRadius, 2);
	}
	virtual double getSpeed() const //[милиметров в милисекунду]
	{
		if(m_dRadius>0.0)
		{
			return 1.0/m_dRadius;
		}
		return DBL_MAX;
	}
	double m_dRadius;
	double m_dStartX;
	double m_dStartY;
	DWORD m_dwMiliseconds;
	EMColoured<IEMColoured> m_colour;
};

class CMainFrame 
	:public CFrameWindowImpl<CMainFrame>
	,public COpenGL<CMainFrame>
	,public CUpdateUI<CMainFrame>
	,public CMessageFilter
	,public CIdleHandler
	
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CMainFrame()
		:m_iX(0)
		,m_iY(0)
		,m_dX(0.0)
		,m_dY(0.0)
		,m_iPixelsAlongHorizontal(0)
		,m_iPixelsAlongVertical(0)
		,m_iMillimetersAlongHorizontal(0)
		,m_iMillimetersAlongVertical(0)
		,m_dHorzPhisicalSizePerPixel(0.0)
		,m_dVertPhisicalSizePerPixel(0.0)
		,m_fontDateTimeProductCompany(0, 0xffff)
		,m_dViewingPositionToNearClippingPlane(0.01) //[mm]
		,m_dViewingPositionToFarClippingPlane(1000000.0) //[mm]
		,m_dViewingPositionToViewPlane(450.0) //[mm]
		,m_dScore(0)
	{
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_lfDateTimeProductCompany);
		m_lfDateTimeProductCompany.lfHeight=-14;
		m_lfDateTimeProductCompany.lfWidth=0;

		formatScore();
	}
	void formatScore()
	{
		m_ssScore.str(std::wstring()); 
		m_ssScore << _T("ОЧКОВ ") << (int)m_dScore << _T(" [мм/c]");
	}
	~CMainFrame()
	{
		while(m_listOfGameObjects.size()>0)
		{
			delete m_listOfGameObjects.front();
			m_listOfGameObjects.pop_front();
		}
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		RedrawWindow();
		return FALSE;
	}

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnAppExit)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		CHAIN_MSG_MAP(COpenGL<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		bHandled = FALSE;
		return 0;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		//<fullscreen>
		//CFrameWindowImplBase::OnDestroy имеет такой код  if((GetStyle() & (WS_CHILD | WS_POPUP)) == 0) ::PostQuitMessage(1);
		//поэтому если не вернуть стиль окна назад на нулевой нивелировав тем самым полноэкранный режим
		//то приложение не будет вызывать ::PostQuitMessage(1); что приведёт к
		//зависанию приложения в GetMessage();
		SetWindowLong(GWL_STYLE, 0);
		//</fullscreen>

		m_fontDateTimeProductCompany.reset();

		bHandled = FALSE;
		return 0;
	}
	LRESULT OnLButtonDown(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled=FALSE;

		DWORD dwMiliseconds=::GetTickCount();
		
		if(::GetFocus()!=m_hWnd)
		{
			SetFocus();
		}

		CPoint point(lParam);

		double observer_x, observer_y, observer_z;
		gluUnProject(point.x, m_iY-point.y, (m_dViewingPositionToViewPlane-m_dViewingPositionToNearClippingPlane)/(m_dViewingPositionToFarClippingPlane-m_dViewingPositionToNearClippingPlane), m_modelMatrix, m_projMatrix, m_viewport, &observer_x, &observer_y, &observer_z);

		std::list<IGameObject*>::reverse_iterator it=m_listOfGameObjects.rbegin();
		while(it!=m_listOfGameObjects.rend())
		{
			if((*it)->test(observer_x, observer_y, dwMiliseconds))
			{
				m_dScore+=1000*(*it)->getSpeed();
				formatScore();

				delete (*it);
				m_listOfGameObjects.erase(--(it.base()) );
				break;
			}
			else
			{
				++it;
			}
		}
		return 0;
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		switch(wParam)
		{
		case VK_ESCAPE:
			{
				PostMessage(WM_CLOSE);
			}
			break;
		}
		return 0;
	}

	LRESULT OnAppExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}
	//----------
	void UpdateFPS()
	{
		const int FRAMEINTERVAL = 333;            // Show FPS every FRAMEINTERVAL milliseconds
		static DWORD nFrames = 0;                  // Number of frames since last update
		static DWORD nLastTick = ::GetTickCount();   // Time of last update
		DWORD nTick = ::GetTickCount();              // Current time
		if(nTick-nLastTick>=FRAMEINTERVAL)
		{	
			double dFPS = 1000.0*nFrames/(nTick-nLastTick);
			nLastTick = nTick;
			nFrames = 0;

			m_ssFPS.str(std::wstring()); 
			m_ssFPS << (int)dFPS << _T(" ФПС");

			m_listOfGameObjects.push_back(new CCircleGameObject(m_dX, m_dY));
		}
		nFrames++;
	}
	void OnInit() 
	{
		m_fontDateTimeProductCompany.init();

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //Background color
	}
	void OnResize(int iX, int iY) 
	{
		if(m_fontDateTimeProductCompany.update(GetDC(), m_lfDateTimeProductCompany))
		{
			short sLength; short sHeight;
			m_fontDateTimeProductCompany.getBitmapDimensions(_T("0123456789"), sLength, sHeight);
			m_fontDateTimeProductCompany.setBitmapHeightShift(sHeight);
		}

		m_iPixelsAlongHorizontal=::GetDeviceCaps(GetDC(), HORZRES);
		m_iPixelsAlongVertical=::GetDeviceCaps(GetDC(), VERTRES);

		m_iMillimetersAlongHorizontal=::GetDeviceCaps(GetDC(), HORZSIZE);
		m_iMillimetersAlongVertical=::GetDeviceCaps(GetDC(), VERTSIZE);

		m_dHorzPhisicalSizePerPixel=1.0*m_iMillimetersAlongHorizontal/m_iPixelsAlongHorizontal; // [mm/pixel]
		m_dVertPhisicalSizePerPixel=1.0*m_iMillimetersAlongVertical/m_iPixelsAlongVertical; // [mm/pixel]

		m_iX=iX; //[pixels]
		m_iY=iY; //[pixels]

		m_dX=m_iX*getHorzPhisicalSizePerPixel(); //[mm]
		m_dY=m_iY*getVertPhisicalSizePerPixel(); //[mm]
		

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();


		glOrtho(-0.5*m_dX, 0.5*m_dX, -0.5*m_dY, 0.5*m_dY, m_dViewingPositionToNearClippingPlane, m_dViewingPositionToFarClippingPlane);

		glViewport(0, 0, m_iX, m_iY);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslated(0.0, 0.0, (0.0-m_dViewingPositionToViewPlane));

		double dTilt=90.0;//[градусы]
		glRotated(dTilt-90.0, 1.0, 0.0, 0.0); //поворот камеры вокруг оси x
		
		double dTurn=0.0;//[градусы]
		glRotated(dTurn, 0.0, 0.0, 1.0); //поворот камеры вокруг оси z

		double dZoomX=1.0;
		double dZoomY=1.0;
		double dZoomZ=1.0;
		//ни в коем случае не менять местами масштабирование с поворотами. 
		//Повороты должны быть ДО масштабирвоания, иначе из за разных масштбаов 
		//вдоль xy и z будут серьёзные проблемы с пропорциями вдоль высоты 
		//относительно плоскости xy
		glScaled(dZoomX, dZoomY, dZoomZ); 

		double dToX=0.0; 
		double dToY=0.0; 
		double dToZ=0.0;
		//геометрический центр тяжести обьекта
		//чисто для того чтобы он сразу попал в поле зрения и его не пришлось разыскивать
		glTranslated(dToX, dToY, dToZ);

		glGetDoublev(GL_MODELVIEW_MATRIX, m_modelMatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix);
		glGetIntegerv(GL_VIEWPORT, m_viewport);
	}
	void OnRender()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		

		std::list<IGameObject*>::iterator it=m_listOfGameObjects.begin();
		while(it!=m_listOfGameObjects.end())
		{
			if((*it)->draw())
			{
				++it;
			}
			else
			{
				delete (*it);
				it=m_listOfGameObjects.erase(it);
			}
		}

		UpdateFPS();

		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_INVERT);

		glRasterPos2d(0.5*m_dX, -0.5*m_dY); 
		m_fontDateTimeProductCompany.setParPer(2, 0);
		m_fontDateTimeProductCompany.print(m_ssFPS.str().c_str());

		glRasterPos2d(-0.5*m_dX, -0.5*m_dY); 
		m_fontDateTimeProductCompany.setParPer(0, 0);
		m_fontDateTimeProductCompany.print(m_ssScore.str().c_str());

		glLogicOp(GL_COPY);
		glDisable(GL_COLOR_LOGIC_OP);

		glFlush();
	}
private:
	int m_iX, m_iY;
	double m_dX, m_dY;
	int m_iPixelsAlongHorizontal, m_iPixelsAlongVertical;
	int m_iMillimetersAlongHorizontal, m_iMillimetersAlongVertical;
	double m_dHorzPhisicalSizePerPixel, m_dVertPhisicalSizePerPixel;
	double getHorzPhisicalSizePerPixel() // [mm/pixel]
	{
		return m_dHorzPhisicalSizePerPixel;
	}
	double getVertPhisicalSizePerPixel() // [mm/pixel]
	{
		return m_dVertPhisicalSizePerPixel;
	}

	double m_dViewingPositionToNearClippingPlane; //[mm]
	double m_dViewingPositionToFarClippingPlane; //[mm]
	double m_dViewingPositionToViewPlane; //[mm]

	LOGFONT m_lfDateTimeProductCompany;
	CGLFont m_fontDateTimeProductCompany;
	std::list<IGameObject*> m_listOfGameObjects;

	double m_modelMatrix[16];
	double m_projMatrix[16];
	int m_viewport[4];
	
	std::wostringstream m_ssFPS;
	std::wostringstream m_ssScore;
	double m_dScore;
};
