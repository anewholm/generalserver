<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="defines">
  <javascript:define name="NO_CALLBACK">undefined</javascript:define>
  <javascript:define name="DEFAULT">undefined</javascript:define>
  <javascript:define name="OPTIONAL">true</javascript:define>

  <javascript:global-function name="NOT_COMPLETE">
    if (window.console) window.console.error("NOT_COMPLETE");
    debugger;
  </javascript:global-function>
  <javascript:global-function name="NOT_CURRENTLY_USED">
    if (window.console) window.console.error("NOT_CURRENTLY_USED");
    debugger;
  </javascript:global-function>

  <!-- note that window.Math is only a static Object, not a Function -->
  <javascript:define name="JS_PRIMITIVES">[String,Boolean,Number]</javascript:define>
  <javascript:define name="JS_BUILTIN_OBJECT_SUBCLASSES">[Array,Date,Function,window.RegExp]</javascript:define>
  <javascript:define name="JS_BUILTIN_OBJECTS">[Object,Array,Date,Function,window.RegExp]</javascript:define>
  <javascript:define name="JS_LIBRARY_OBJECTS">[window.jQuery]</javascript:define>
  <javascript:define name="HTML_BUILTIN_OBJECTS">[window.Document,window.Node]</javascript:define>
  <javascript:define name="JS_BASE_CHECK_TYPES">JS_PRIMITIVES.concat(JS_BUILTIN_OBJECT_SUBCLASSES).concat(HTML_BUILTIN_OBJECTS).concat(JS_LIBRARY_OBJECTS)</javascript:define>
</javascript:code>