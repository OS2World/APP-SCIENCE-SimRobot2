#ifndef __SRDATAVIEW_H__
#define __SRDATAVIEW_H__

#include "srBaseView.h"

#ifdef __WIN16__
typedef char __huge* PCHAR;
#else
typedef char* PCHAR;
#endif

class _DOCVIEWCLASS TDataView : public TBaseView
{
  private:
    BOOL m_bRepaint;
    TRect m_Rect;
    TBitmap* m_pBitmap;
  protected:
    STRING m_sObject;
  public:
    TDataView(TDoc& doc,TResId Menu,TWindow* parent = 0);
    ~TDataView() {delete m_pBitmap;}
    void WriteLayout(fstream& f);
    void ReadLayout(fstream& f,int nVersion);
    void SetupWindow();
    void Paint(TDC& dc, BOOL, TRect&);
    void Invalidate();
  protected:
    void EvSysColorChange();
    BOOL SetDocTitle(const char*, int);
    virtual void OnPaint(TDC&) = 0;
    virtual PCHAR OnWrite(HANDLE& handle) {return 0;}
    void EvSize(UINT sizeType,TSize& size);
    virtual BOOL OnSuccess() = 0;
    BOOL VnSuccess(SIMULATION*);
    BOOL VnRepaint();
    void CmEditCopy();
    void CeEditCopy(TCommandEnabler& ce);
  DECLARE_RESPONSE_TABLE(TDataView);
};

#endif

