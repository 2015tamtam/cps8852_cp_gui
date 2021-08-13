// mtp_test_guiDlg.cpp : implementation file
//


#include "stdafx.h"
#include "mtp_test_gui.h"
#include "mtp_test_guiDlg.h"

#include <io.h>
#include <fcntl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMtp_test_guiDlg dialog

CMtp_test_guiDlg::CMtp_test_guiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMtp_test_guiDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMtp_test_guiDlg)
	m_addr = _T("");
	m_data = _T("");
	m_mtp_case = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// m_addr.Format("%08X", 0x20000410);
	// m_data.Format("%08X", 0);
	m_addr.Format("%08X", 0x4001F000); // remap_enable
	m_data.Format("%08X", 0xFFFFFF00);
	m_mtp_case.Format("%02X", 0x00);

	opentty();

	file_name = NULL;
	i2c_open();
	i2c_power_off();
}

void CMtp_test_guiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMtp_test_guiDlg)
	DDX_Control(pDX, IDC_PROGRESS1, m_pro);
	DDX_Text(pDX, IDC_EDIT1, m_addr);
	DDX_Text(pDX, IDC_EDIT2, m_data);
	DDX_Text(pDX, IDC_EDIT3, m_mtp_case);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMtp_test_guiDlg, CDialog)
	//{{AFX_MSG_MAP(CMtp_test_guiDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, opentty)
	ON_BN_CLICKED(IDC_BUTTON2, close_tty)
	ON_BN_CLICKED(IDC_BUTTON3, load_hex)
	ON_BN_CLICKED(IDC_BUTTON4, mtp_cp1_test) // this is "load_to_eFlash" process
	ON_BN_CLICKED(IDC_BUTTON5, mtp_cp2_test)
	ON_BN_CLICKED(IDC_BUTTON6, mtp_cp_test)
	ON_BN_CLICKED(IDC_BUTTON7, read_all_mtp)
	ON_BN_CLICKED(IDC_BUTTON8, mem_read)
	ON_BN_CLICKED(IDC_BUTTON10, i2c_check)
	ON_BN_CLICKED(IDC_BUTTON11, mcu_reset_halt)
	ON_BN_CLICKED(IDC_BUTTON12, sys_reset)
	ON_BN_CLICKED(IDC_BUTTON9, mem_write)
	ON_BN_CLICKED(IDC_BUTTON13, mtp_case_run)
	ON_BN_CLICKED(IDC_BUTTON14, OnButton14)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMtp_test_guiDlg message handlers

BOOL CMtp_test_guiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMtp_test_guiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMtp_test_guiDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMtp_test_guiDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMtp_test_guiDlg::opentty() 
{
	// TODO: Add your control notification handler code here
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle,_O_TEXT);
	FILE * hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
}

void CMtp_test_guiDlg::close_tty() 
{
	// TODO: Add your control notification handler code here
	FreeConsole();
}

void CMtp_test_guiDlg::load_hex() 
{
	// TODO: Add your control notification handler code here	
	TCHAR szFilter[] = _T("hex文件(*.hex)|*.hex|所有文件(*.*)|*.*||");
	// 构造打开文件对话框   
	CFileDialog fileDlg(TRUE, _T("txt"), NULL, 0, szFilter, this);
	
	CTime tm = CTime::GetCurrentTime();
	CString strtm= tm.Format("%Y-%m-%d %H:%M:%S");
	
	file_name = "";
	
	// 显示打开文件对话框   
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“打开”按钮，则将选择的文件路径显示到编辑框里   
		filepath  = fileDlg.GetPathName();
		file_name = filepath.GetBuffer(100);
		printf("load_hex : %s\n", file_name);
	}
}

void CMtp_test_guiDlg::mtp_cp1_test() 
{
	// TODO: Add your control notification handler code here
	int		len = 0;
	int     fail_flag = 0;
    uint32  app_buf[(16*1024)];
	uint32  tmp_buf[(16*1024)];

	i2c_power_on();

	m_pro.SetRange(0, 100);
	m_pro.SetStep(1);
	m_pro.SetPos(0);

	printf("load_hex : %s\n", file_name);
	
	if(NULL != file_name){
		len = hex2bin(file_name, (unsigned char *)app_buf);
	}
	else {
		AfxMessageBox("Can't Load hex file !!!");
	}
	
    if(0 == len){
		fail_flag = 1;
	}
	else {
		i2c_wr_word(0xFFFFFF00, 0x0000000E);
		i2c_wr_word(0x40014018, 0x00009A6E);
		i2c_wr_word(0x40014014, 0x00010000);

		i2c_wr_word(0x40015010,         25);
		i2c_wr_word(0x4001501C, 0xFFFFFFFF);

		i2c_wr_mem_word(0x00000000, app_buf, len/4); // load to eFlash@0x0

		i2c_wr_word(0x0000C39C, 0x00000030); // i2c slave address
		i2c_wr_word(0x0000C3F8, 0x5724E185); // mcu  start flag
		i2c_wr_word(0x0000C3FC, 0x7455874E); // trim start flag

		m_pro.SetPos(50);

		i2c_rd_mem_word(0x00000000, tmp_buf, len/4); // read from eFlash@0x0
		
		m_pro.SetPos(100);
	
		for(int i=0; i<len/4; i++){
			if(app_buf[i] != tmp_buf[i]){
				fail_flag = 1;
				break;
			}
		}
	}

	if(fail_flag  == 0){
		AfxMessageBox("Download to eFlash Pass");
		sys_reset();
	}
	else
		AfxMessageBox("Download to eFlash Fail");
}

void CMtp_test_guiDlg::mtp_cp2_test() 
{
	// TODO: Add your control notification handler code here
	int		len = 0;
	int     fail_flag = 0;
    uint32  app_buf[(16*1024)];
	uint32  tmp_buf[(16*1024)];

	i2c_power_on();

	m_pro.SetRange(0, 100);
	m_pro.SetStep(1);
	m_pro.SetPos(0);

	printf("load_hex : %s\n", file_name);
	
	if(NULL != file_name){
		len = hex2bin(file_name, (unsigned char *)app_buf);
	}
	else {
		AfxMessageBox("Can't Load hex file !!!");
	}
	
    if(0 == len){
		fail_flag = 1;
	}
	else {
		i2c_wr_word(0xFFFFFF00, 0x0000000E);
		i2c_wr_word(0x40014018, 0x00009A6E);
		i2c_wr_word(0x40014014, 0x00010000);

		i2c_wr_word(0x40015010,         25);
		i2c_wr_word(0x4001501C, 0xFFFFFFFF);

		i2c_wr_mem_word(0x20000000, app_buf, len/4);	// load to SRAM@0x2000_0000
//	printf("load_hex : %s to SRAM ok, read it back checking...\n", file_name);

		i2c_wr_word(0x0000C39C, 0x00000030); // i2c slave address
		i2c_wr_word(0x0000C3F8, 0x5724E185); // mcu  start flag
		i2c_wr_word(0x0000C3FC, 0x7455874E); // trim start flag
		m_pro.SetPos(50);	

		i2c_rd_mem_word(0x20000000, tmp_buf, len/4);	// read from SRAM@0x2000_0000
//	printf("Read %s from SRAM back checking...\n", file_name);
		m_pro.SetPos(100);
	
		for(int i=0; i<len/4; i++){
			if(app_buf[i] != tmp_buf[i]){
				fail_flag = 1;
				break;
			}
		}
	}

	if(fail_flag  == 0){
		AfxMessageBox("Download to sRAM Pass");	
		sys_reset();
		
	}
	else {
		AfxMessageBox("Download to sRAM Fail");	
	}
}

void CMtp_test_guiDlg::mtp_cp_test() 
{
	// TODO: Add your control notification handler code here
}

void CMtp_test_guiDlg::read_all_mtp() 
{

}

void CMtp_test_guiDlg::mem_read() 
{
	// TODO: Add your control notification handler code here
	int addr;
	int data;

	addr = get_m_addr();
	data = get_m_data();

	data = i2c_rd_word(addr);

	set_m_data(data);

	printf("---> mem_rd %08X : %08X\n", addr, data);
}


void CMtp_test_guiDlg::mem_write() 
{
	// TODO: Add your control notification handler code here
	
	int addr;
	int data;
	
	addr = get_m_addr();
	data = get_m_data();
	
	i2c_wr_word(addr, data);
	
	set_m_data(data);
	
	printf("---> mem_wr %08X : %08X\n", addr, data);
}


void CMtp_test_guiDlg::i2c_check() 
{
	// TODO: Add your control notification handler code here
	int fail = 0;
	i2c_power_on();

    i2c_wr_word(0xFFFFFF00, 0x0000000E);
    i2c_wr_word(0x40014018, 0x00009A6E);

	if(1 != i2c_rd_word(0x40014018)){
		fail = 1;
	}

	if(fail){
		printf("---> i2c check fail\n");
	}
	else {
		printf("---> i2c check pass\n");
	}
}

void CMtp_test_guiDlg::mcu_reset_halt() 
{
	// TODO: Add your control notification handler code here
	int fail = 0;
	i2c_power_on();
	
    i2c_wr_word(0xFFFFFF00, 0x0000000E);
    i2c_wr_word(0x40014018, 0x00009A6E);
    i2c_wr_word(0x40014014, 0x00010000);
	
	if(0x00010000 != i2c_rd_word(0x40014014)){
		fail = 1;
	}

	if(fail){
		printf("---> mcu reset & halt fail\n");
	}
	else {
		printf("---> mcu reset & halt pass\n");
	}
}

void CMtp_test_guiDlg::sys_reset() 
{
	// TODO: Add your control notification handler code here
	int fail = 0;
	i2c_power_on();

    i2c_wr_word(0xFFFFFF00, 0x0000000E);
    i2c_wr_word(0x40014018, 0x00009A6E);
    i2c_wr_word(0x40014014, 0x01000000);

	if(1 == i2c_rd_word(0x40014018)){
		fail = 1;
	}

	_delayms(100);
	
	
    i2c_wr_word(0xFFFFFF00, 0x0000000E);
    i2c_wr_word(0x40014018, 0x00009A6E);
	
	if(0 == i2c_rd_word(0x40014018)){
		fail = 1;
	}

	if(1 == fail){
		printf("---> sys_reset fail\n");
	}
	else {
		printf("---> sys_reset pass\n");
	}
}


void CMtp_test_guiDlg::mtp_case_run() 
{

}

void CMtp_test_guiDlg::OnButton14() 
{

}
