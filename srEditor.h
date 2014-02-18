#ifndef __SREDITOR_H__
#define __SREDITOR_H__

#include "srDoc.h"

class _DOCVIEWCLASS TEditor : public TEditView
{
  private:
    TMenu m_Menu;
    TPopupMenu m_PopupMenu;
  public:
    TEditor(TDoc& doc, TWindow *parent = 0);
    ~TEditor();
    void WriteLayout(fstream& f);
    void ReadLayout(fstream& f,int nVersion);
    static const char far* StaticName() {return "Editor";}
    const char far* GetViewName(){return StaticName();}
    TDoc* m_pDoc;
  protected:
    void SetupWindow();
    BOOL SetDocTitle(const char*,int);
    BOOL VnCommit(BOOL force);
    BOOL VnError(TPoint* pt);
    void EvChar(UINT nChar,UINT nRepCnt,UINT nFlags);
    void EvRButtonDown(UINT,TPoint&);
    void CmHelpOnTopic();
#ifndef __OS2__
  private:
    void FixBuffer();
  public:
    BOOL Create();
  protected:
    BOOL VnRevert(BOOL clear);
    BOOL VnDocClosed(int omode);
#else
    void EvDropFiles(TDropInfo dropInfo);
#endif
  DECLARE_RESPONSE_TABLE(TEditor);
};

#endif
