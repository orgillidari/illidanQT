#include "XOpenGLWindow.h"


XOpenGLWindow::XOpenGLWindow(QWidget *parent)
	:QOpenGLWidget(parent)
{
	//ȫ��
	m_bFullScreen = true;
	m_Width = 0;
	m_Height = 0;

	m_TimerID = 0;

	m_LastMouseX = 0;
	m_LastMouseY = 0;

	//����OpenGL�İ汾��Ϣ
	QSurfaceFormat format;
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setVersion(3, 3);
	setFormat(format);
}

XOpenGLWindow::~XOpenGLWindow()
{
	killTimer(m_TimerID);
}

void XOpenGLWindow::initializeGL()
{
	//Init
	XManager::GetRef().Init();

	//������ʱ��
	m_TimerID = startTimer(30);
}

void XOpenGLWindow::resizeGL(int width, int height)
{
	m_Width = width;
	m_Height = height;

	//Resize
	XManager::GetRef().m_Render->Resize(m_Width, m_Height);
}

void XOpenGLWindow::paintGL()
{
//Logic
	//����
	XManager::GetRef().m_Camera->OnUpdate();
	

//Render
	XManager::GetRef().m_Render->Render(XManager::GetRef(), m_Width, m_Height);
	XManager::GetRef().m_Skybox->Render(XManager::GetRef(), m_Width, m_Height);
}

void XOpenGLWindow::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		XManager::GetRef().m_Camera->OnRightButtonDown(event->x(), event->y());
		ShowCursor(FALSE);
		m_LastMouseX = event->x();
		m_LastMouseY = event->y();
	}
}

void XOpenGLWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		XManager::GetRef().m_Camera->OnRightButtonUp(event->x(), event->y());
		ShowCursor(TRUE);
		QPoint rect = this->pos();
		SetCursorPos(rect.x() + m_LastMouseX, rect.y() + m_LastMouseY);
	}
}

void XOpenGLWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
}

void XOpenGLWindow::mouseMoveEvent(QMouseEvent *event)
{
	XManager::GetRef().m_Camera->OnMouseMove(event->x(), event->y());
}

#ifndef QT_NO_WHEELEVENT
void XOpenGLWindow::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0)
	{
		XManager::GetRef().m_Camera->OnZoomIn();
	}
	else
	{
		XManager::GetRef().m_Camera->OnZoomOut();
	}
}

#endif

void XOpenGLWindow::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_W:
	case Qt::Key_S:
	case Qt::Key_A:
	case Qt::Key_D:
		XManager::GetRef().m_Camera->OnKeyDown(event->key());
		break;
	case Qt::Key_1:
		if (event->modifiers() == Qt::AltModifier)
			close();
		break;
	default:
		QWidget::keyPressEvent(event);
		break;
	}
}

void XOpenGLWindow::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_W:
	case Qt::Key_S:
	case Qt::Key_A:
	case Qt::Key_D:
		XManager::GetRef().m_Camera->OnKeyUp(event->key());
		break;
	default:
		QWidget::keyPressEvent(event);
		break;
	}
}

void XOpenGLWindow::timerEvent(QTimerEvent * event)
{
	repaint();
}