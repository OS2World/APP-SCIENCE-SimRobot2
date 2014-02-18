#include "pch.h"
#include "srEditor.h"
#include "srApp.h"

DEFINE_DOC_TEMPLATE_CLASS(TDoc,TEditor,EditorTemplate);
EditorTemplate editorTpl("Scenes (*.scn)","*.scn",0,"scn",
                          dtAutoDelete |
                          dtFileMustExist |
                          dtOverwritePrompt |
                          dtSingleView|
                          dtUpdateDir);

DEFINE_RESPONSE_TABLE1(TEditor,TEditView)
#ifndef __OS2__
  EV_VN_DOCCLOSED,
  EV_VN_REVERT,
#else
  EV_WM_DROPFILES,
#endif
  EV_VN_COMMIT,
  EV_VN_ERROR,
  EV_WM_CHAR,
  EV_WM_RBUTTONDOWN,
  EV_COMMAND(CM_HELPONTOPIC,CmHelpOnTopic),
END_RESPONSE_TABLE;

TEditor::TEditor(TDoc& doc,TWindow *parent)
: TEditView(doc,parent),
  m_Menu(*GetModule(),IDP_EDITOR),
#ifdef __OS2__
  m_PopupMenu(m_Menu),
#else
  m_PopupMenu(m_Menu.GetSubMenu(0)),
#endif
  m_pDoc(&doc)
{
  SetViewMenu(new TMenuDescr(IDM_EDITOR,0,3,0,0,0,1));
}

TEditor::~TEditor()
{
  TApplication* pApp = GetApplication();
  if(pApp)
  {
    TWindow* pWnd = pApp->GetMainWindow();
    if(pWnd)
      pWnd->SetCaption("SimRobot");
  }
}

BOOL TEditor::VnCommit(BOOL force)
{
  int nTop;
  UINT nStart,nStop;
  nTop = GetFirstVisibleLine();
  GetSelection(nStart,nStop);
  BOOL bResult = TEditView::VnCommit(force);
  Scroll(0,nTop - GetFirstVisibleLine());
  SetSelection(nStart,nStop);
  return bResult;
}

#ifndef __OS2__
void TEditor::FixBuffer()
{
  char* p = LockBuffer();
  int nLen = strlen(p),
      nCount = 0;
  if(nLen)
  {
    for(int i = nLen-1; i >= 0; i--)
      if(p[i] == '\n' && (!i || p[i-1] != '\r'))
        nCount++;
    UnlockBuffer(p,FALSE);
    p = LockBuffer(nLen+nCount+1);
    for(i = nLen-1; i >= 0; i--)
      if(p[i] == '\n' && (!i || p[i-1] != '\r'))
      {
        memmove(&p[i+1],&p[i],nLen-i+1);
        p[i] = '\r';
        nLen++;
      }
    UnlockBuffer(p,TRUE);
  }
}

BOOL TEditor::Create()
{
  if(TEditView::Create())
  {
    FixBuffer();
    return TRUE;
  }
  else
    return FALSE;
}

BOOL TEditor::VnRevert(BOOL clear)
{
  if(TEditView::VnRevert(clear))
  {
    FixBuffer();
    return TRUE;
  }
  else
    return FALSE;
}

BOOL TEditor::VnDocClosed(int omode)
{
  if(TEditView::VnDocClosed(omode))
  {
    FixBuffer();
    return TRUE;
  }
  else
    return FALSE;
}
#endif

void TEditor::WriteLayout(fstream& f)
{
  (dynamic_cast<TMDIBaseChild*>(Parent))->WriteLayout(f);
  int nTop;
  UINT nStart,nStop;
  nTop = GetFirstVisibleLine();
  GetSelection(nStart,nStop);
  f << nTop << " "
    << nStart << " "
    << nStop << "\n";
}

void TEditor::ReadLayout(fstream& f,int)
{
  int nTop;
  UINT nStart,nStop;
  f >> nTop >> nStart >> nStop;
  Scroll(0,nTop - GetFirstVisibleLine());
  SetSelection(nStart,nStop);
}

void TEditor::SetupWindow()
{
  TEditView::SetupWindow();
#ifdef __OS2__
  TFont mono("System Monospaced",16,8);
  CONST_CAST(TEditor*,this)->DefWindowProc(MLM_SETFONT,
             (WPARAM) (HFONT) mono);
  SetFocus();
#else
  HGDIOBJ font = GetStockObject(SYSTEM_FIXED_FONT);
  SendMessage(WM_SETFONT, (UINT)font, 0L);
#endif
}

BOOL TEditor::SetDocTitle(const char*,int index)
{
  if (index >= 0)
  {
    Parent->SetCaption(GetViewName());
    STRING s("SimRobot");
    if(m_pDoc->GetDocPath())
      s = s + STRING(" - ") + STRING(m_pDoc->GetDocPath());
    else
      s = s + STRING(" - Untitled");
    GetApplication()->GetMainWindow()->SetCaption(s);
  }
  return TRUE;
}

BOOL TEditor::VnError(TPoint* pt)
{
  if (IsIconic())
    ShowWindow(SW_SHOWNORMAL);
  SetFocus();
  if (pt->y >= 0)
  {
    int p = GetLineIndex(pt->y - 1) + pt->x - 1;
    SetSelection(p,p);
  }
  return TRUE;
}

#ifdef __OS2__
#define IS_BACKTAB (nChar == VK_BACKTAB)
#else
#define IS_BACKTAB (GetKeyState(VK_SHIFT) & 0x8000)
#endif

void TEditor::EvChar(UINT nChar,UINT nRepCnt,UINT nFlags)
{
  static char Buffer[256];
  unsigned nStart,nStop;
  switch (nChar)
  {
    case VK_RETURN:
    {
// Auto-indend a new line like the current line
      GetSelection(nStart,nStop);
      int nPos = nStart - GetLineIndex(GetLineFromPos(nStart));
      Buffer[0] = '\r';
      Buffer[1] = '\n';
      GetLine(Buffer+2,sizeof(Buffer)-2,GetLineFromPos(nStart));
      int i = 2;
      while (i-2 < nPos && (Buffer[i] == ' ' || Buffer[i] == '\t'))
        i++;
      Buffer[i] = 0;
      Insert(Buffer);
      break;
    }
    case VK_TAB:
#ifdef __OS2__
    case VK_BACKTAB:
#endif
    {
// indend or unindent the selected lines
      GetSelection(nStart,nStop);
      int lineStart = GetLineFromPos(nStart);
      if (nStart != nStop)
      {
        int lineStop = GetLineFromPos(nStop);
            nStart = GetLineIndex(lineStart);
        if(GetLineIndex(lineStop) != nStop)
          lineStop++;
        nStop = GetLineIndex(lineStop);
        SetSelection(nStart,nStop);
        char *pData;
        if (IS_BACKTAB)
        {
// unindent block
          pData = new char [nStop-nStart+lineStop-lineStart+1];
          pData[0] = 0;
          for(int i = lineStart; i < lineStop; i++)
          {
            GetLine(Buffer,sizeof(Buffer),i);
            if(Buffer[0] == ' ')
            {
              strcat(pData,Buffer+1);
              nStop--;
            }
            else
              strcat(pData,Buffer);
#ifndef __OS2__
            strcat(pData,"\r\n");
#endif
          }
          Insert(pData);
          SetSelection(nStart,nStop);
        }
        else
        {
// indend block
          int nSize = nStop-nStart+2*(lineStop-lineStart)+1,
                      nOffset = 0;
          pData = new char [nSize];
          for(int i = lineStart; i < lineStop; i++)
          {
            pData[nOffset++] = ' ';
            GetLine(pData+nOffset,nSize-nOffset,i);
            nOffset += strlen(pData+nOffset);
#ifndef __OS2__
            pData[nOffset++] = '\r';
            pData[nOffset++] = '\n';
#endif
          }
          pData[nOffset] = 0;
          Insert(pData);
          SetSelection(nStart,nStop+lineStop-lineStart);
        }
        delete [] pData;
      }
      else
      {
// Insert as much spaces as one tab would be
        int nPos = nStart - GetLineIndex(lineStart),
            nSize = ((nPos+8) / 8) * 8 - nPos;
        char *pSpace = "        ";
        Insert(pSpace+(8-nSize));
      }
      break;
     }
    default:
      TEditView::EvChar(nChar, nRepCnt, nFlags);
  }
}

void TEditor::EvRButtonDown(UINT,TPoint& point)
{
  Parent->SetFocus();
  SetFocus();
  TPoint lp = point;
  ClientToScreen(lp);
  TFrame* pFrame = dynamic_cast<TFrame*>(GetApplication()->GetMainWindow());
#ifndef __OS2__
  m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,lp,0,*pFrame);
#else
  pFrame->EvInitMenuPopup(m_PopupMenu,0,FALSE);
  pFrame->ScreenToClient(lp);
  m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,lp,*pFrame);
}

void TEditor::EvDropFiles(TDropInfo dropInfo)
{
  (dynamic_cast<TApp*>(GetApplication()))->EvDropFiles(dropInfo);
#endif
}

void TEditor::CmHelpOnTopic()
{
  static char buf[256];

  UINT nStart,nStop;
  GetSelection(nStart,nStop);
  nStart -= GetLineIndex(-1);
  GetLine(buf,sizeof(buf),GetLineFromPos(-1));
  int nLength = strlen(buf);
  nStop = nStart;
  while (nStart+1 > 0 && (isalpha(buf[nStart]) || buf[nStart] == '_'))
    nStart--;
  while (nStop < nLength && (isalpha(buf[nStop]) || buf[nStop] == '_'))
    nStop++;
  buf[nStop] = 0;
  (dynamic_cast<TFrame*>(GetApplication()->GetMainWindow()))
    ->ShowHelp(&buf[nStart+1]);
}

