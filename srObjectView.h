#ifndef __SROBJECTVIEW_H__
#define __SROBJECTVIEW_H__

#include "srDataView.h"

class _DOCVIEWCLASS TObjectView : public TDataView
{
  private:
    SIM3DOBJECT m_pObject;
    int m_nScale,
        m_nDistortion,
        m_nDetail;
    BOOL m_bHidden;
  public:
    BOOL m_bTracking;
    TObjectView(TDoc& doc,TWindow* parent = 0);
    static TObjectView* New(TDoc&);
    void WriteLayout(fstream& f);
    void ReadLayout(fstream& f,int nVersion);
    static const char far* StaticName() {return "Object";}
    const char far* GetViewName(){return StaticName();}
  protected:
    void SetupWindow();
    void OnPaint(TDC& dc);
    BOOL OnSuccess();
    void CmZoom(WPARAM nId);
    void CeZoom(TCommandEnabler& ce);
    void CmDistortion(WPARAM nId);
    void CeDistortion(TCommandEnabler& ce);
    void CmDetaillevel(WPARAM nId);
    void CeDetaillevel(TCommandEnabler& ce);
    void CmHidden();
    void CeHidden(TCommandEnabler& ce);
    void CmTurn();
    void CeTurn(TCommandEnabler& ce);
    void EvLButtonDown(UINT,TPoint& point);
  DECLARE_RESPONSE_TABLE(TObjectView);
};

#endif
