<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="controller-clipboard">
  <javascript:global-variable name="gs_clipboard_object"/>
  <javascript:global-variable name="gs_clipboard_state"/>
  <javascript:define name="GS_CLIP_COPY">1</javascript:define>
  <javascript:define name="GS_CLIP_CUT">2</javascript:define>

  <javascript:global-function name="gs_clipboard_copy" parameters="oObject">
    gs_clipboard_state  = GS_CLIP_COPY;
    gs_clipboard_object = oObject;
    new JOO(".gs-enabled-with-clipboard").removeClass("gs-disabled");
    new JOO(document).trigger("clipboardpopulated", [gs_clipboard_object]);
  </javascript:global-function>

  <javascript:global-function name="gs_clipboard_cut" parameters="oObject">
    gs_clipboard_state  = GS_CLIP_CUT;
    gs_clipboard_object = oObject;
    new JOO(".gs-enabled-with-clipboard").removeClass("gs-disabled");
    new JOO(document).trigger("clipboardpopulated", [gs_clipboard_object]);
  </javascript:global-function>
</javascript:code>