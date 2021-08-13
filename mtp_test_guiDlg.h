// mtp_test_guiDlg.h : header file
//

#if !defined(AFX_MTP_TEST_GUIDLG_H__03A231CE_636F_4AD2_9468_0C491CB80299__INCLUDED_)
#define AFX_MTP_TEST_GUIDLG_H__03A231CE_636F_4AD2_9468_0C491CB80299__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <windows.h>

extern "C" {
#include "setupapi.h"
#include "hiddll.h"
}

typedef unsigned        char uint8_t;
typedef unsigned short  int uint16_t;
typedef unsigned        int uint32_t;

#include "usb_lib.h"
#include "i2c_lib.h"
#include "hex_lib.h"
#include "app_lib.h"
#include "i2c_base.h"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

/////////////////////////////////////////////////////////////////////////////
// CMtp_test_guiDlg dialog

class CMtp_test_guiDlg : public CDialog
{
// Construction
public:
	CMtp_test_guiDlg(CWnd* pParent = NULL);	// standard constructor
	Capp			app;
	CString			filepath;
	char*			file_name;

// Dialog Data
	//{{AFX_DATA(CMtp_test_guiDlg)
	enum { IDD = IDD_MTP_TEST_GUI_DIALOG };
	CProgressCtrl	m_pro;
	CString	m_addr;
	CString	m_data;
	CString	m_mtp_case;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMtp_test_guiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	int get_m_addr(void){
		int addr;

		UpdateData(TRUE);
		addr = strtoul(m_addr, NULL, 16);
		UpdateData(FALSE);

		return addr;
	}

	int get_m_data(void){
		int data;
		
		UpdateData(TRUE);
		data = strtoul(m_data, NULL, 16);
		UpdateData(FALSE);
		
		return data;
	}

	int set_m_addr(int data){
		UpdateData(TRUE);
		m_addr.Format("%08X", data);
		UpdateData(FALSE);
		return data;
	}

	int set_m_data(int data){
		UpdateData(TRUE);
		m_data.Format("%08X", data);
		UpdateData(FALSE);
		return data;
	}

	int i2c_wr_mem_word (uint32 addr, uint32 *dat, int len){
		return app.i2c_wr_mem_word(addr, dat, len);
	}
	
	int i2c_rd_mem_word (uint32 addr, uint32  *dat, int len){
		return app.i2c_rd_mem_word(addr, dat, len);
	}
	
	int i2c_wr_word(uint32 addr, uint32 data){
		return app.i2c_wr_word(addr, data);
	}
	
	uint32 i2c_rd_word(uint32 addr){
		return app.i2c_rd_word(addr);
	}
	
	int i2c_open(void){
		init_srand();
		app.open_usb();
		return 0;
	}
	
	int i2c_close(void){
		app.close_usb();
		return 0;
	}
	
	int _delayns(int dat){
		Sleep(dat);
		return 0;
	}
	int _delayus(int dat){
		Sleep(dat);
		return 0;
	}
	int _delayms(int dat){
		Sleep(dat);
		return 0;
	}
	
	int _over(char *dat){
		return 0;
	}
	
	int i2c_power_on(){
		return app.i2c_power_on();
	}
	
	int i2c_power_off(){
		return app.i2c_power_off();
	}
	
	int test_pass(){printf("---> PASS\n");return 0;};
	int test_fail(){printf("---> FAIL\n");return 0;};
	

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMtp_test_guiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void opentty();
	afx_msg void close_tty();
	afx_msg void load_hex();
	afx_msg void mtp_cp1_test();
	afx_msg void mtp_cp2_test();
	afx_msg void mtp_cp_test();
	afx_msg void read_all_mtp();
	afx_msg void mem_read();
	afx_msg void i2c_check();
	afx_msg void mcu_reset_halt();
	afx_msg void sys_reset();
	afx_msg void mem_write();
	afx_msg void mtp_case_run();
	afx_msg void OnButton14();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MTP_TEST_GUIDLG_H__03A231CE_636F_4AD2_9468_0C491CB80299__INCLUDED_)
