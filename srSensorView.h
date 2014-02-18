#ifndef __SRSENSORVIEW_H__
#define __SRSENSORVIEW_H__

#include "srDataView.h"

class _DOCVIEWCLASS TSensorView : public TDataView
{
  private:
    SENSORPORT m_spSensor;
    int m_nType;
    BOOL m_bGrid;
    double m_dBright;
    int m_nDim,
        m_nDimSize[3],
        m_xSize,
        m_ySize,
        m_yAdd;
  public:
    TSensorView(TDoc& doc,TWindow* parent = 0);
    static TSensorView* New(TDoc&);
    void WriteLayout(fstream& f);
    void ReadLayout(fstream& f,int nVersion);
    static const char far* StaticName() {return "Sensor";}
    const char far* GetViewName(){return StaticName();}
  protected:
    void OnPaint(TDC& dc);
    BOOL OnSuccess();
    virtual PCHAR OnWrite(HANDLE& handle);
    void CmViewer(WPARAM nId);
    void CeLine(TCommandEnabler& ce);
    void CeColumn(TCommandEnabler& ce);
    void CeGrey(TCommandEnabler& ce);
    void CeColor(TCommandEnabler& ce);
    void CeStereo(TCommandEnabler& ce);
    void CeDrawing(TCommandEnabler& ce);
    void CmGrid();
    void CeGrid(TCommandEnabler& ce);
    void CmBright(WPARAM nId);
    void CeBright(TCommandEnabler& ce);
  private:
    void Grid2D(TDC& dc);
    void Grid3D(TDC& dc);
    void Constrains(PSHORTREAL pDepth,INTEGER* pConstrains,
                    BOOL* pVisible,REAL xDepthStep);
    TPoint Project2D(int x,int y,SHORTREAL z);
    void GreyViewer(TDC& dc,PSHORTREAL pData);
    void ColorViewer(TDC& dc,PSHORTREAL pData);
    void LineViewer(TDC& dc,PSHORTREAL pData);
    void ColumnViewer(TDC& dc,PSHORTREAL pData);
    void SirdsViewer(TDC& dc,PSHORTREAL pData);
    void DrawingViewer(TDC& dc,PSHORTREAL pData);
  DECLARE_RESPONSE_TABLE(TSensorView);
};

#endif

