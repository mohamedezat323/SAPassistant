If Not IsObject(application) Then
   Set SapGuiAuto  = GetObject("SAPGUI")
   Set application = SapGuiAuto.GetScriptingEngine
End If
If Not IsObject(connection) Then
   Set connection = application.Children(0)
End If
If Not IsObject(session) Then
   Set session    = connection.Children(0)
End If
If IsObject(WScript) Then
   WScript.ConnectObject session,     "on"
   WScript.ConnectObject application, "on"
End If
session.findById("wnd[0]").maximize
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").text = "31220900"
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").setFocus
session.findById("wnd[0]/usr/ctxtRESB-SAKNR").caretPosition = 8
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
session.findById("wnd[0]").sendVKey 0
