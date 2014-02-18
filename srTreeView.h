#ifndef __SRTREEVIEW_H__
#define __SRTREEVIEW_H__

#include "srBaseView.h"

class _DOCVIEWCLASS TTreeView : public TBaseView
{
  private:
    int m_xMax;
    TDC* m_pDC;
    TBitmap* m_pBmpIcons;
    TMemoryDC* m_pBmpDC;
    int m_nTemplate;
    void BitmapBackground();
    STRING Add(STRING s1,STRING s2);
    void TextOut(int x,int& y,int& y2,STRING s,int nIcon,STRING Name);
    void TTreeView::EnumObjects(SIM3DOBJECT s,int x,int& y,int& y2,
                                STRING Name = "");
    STRING TTreeView::GetName(SIM3DOBJECT s,int ySearch,int& y,
                              STRING Name = "");
  public:
    TTreeView(TDoc& doc,TWindow* parent = 0);
    ~TTreeView();
    static TTreeView* New(TDoc&);
    void WriteLayout(fstream& f);
    void ReadLayout(fstream& f,int nVersion);
    void Paint(TDC& dc, BOOL, TRect&);
    static const char far* StaticName() {return "Tree";}
    const char far* GetViewName(){return StaticName();}
  protected:
    virtual void SetupWindow();
    void EvSysColorChange();
    BOOL EvEraseBkgnd(HDC);
    BOOL SetDocTitle(const char*, int);
    void EvLButtonDown(UINT,TPoint&);
    void EvLButtonDblClk(UINT,TPoint&);
    void CmOpenView();
    void CeOpenView(TCommandEnabler& ce);
    BOOL VnSuccess(SIMULATION* pSim);
    void CmHelpOnObject();
  DECLARE_RESPONSE_TABLE(TTreeView);
};

#endif
