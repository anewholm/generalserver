<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:dependency uri="/resources/shared/jquery/plugins/splitter/js/jquery.splitter-0.14.0.js"/>

  <javascript:object>
    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      //Frameset @elements = interface:VerticalFrameset | interface:HorizontalFrameset
      //DisplayObjects have the local-name() on them: gs-local-name-HorizontalFrameset
      if (this.startPosition || this.limit || this.displayClass == "split") {
        //TODO: .isA() should work on this.localName, not on the class on the DisplayObject
        //TODO: split() is not loaded on to JQuery, and thus DisplayObject
        //because it is loaded after here
        j1DisplayObject.elements.split({
          orientation: (this.isA("VerticalFrameset") ? "vertical" : "horizontal"),
          position:    (this.startPosition ? parseInt(this.startPosition) : 300),
          limit:       (this.limit ? parseInt(this.limit) : 150)
        });
      }
    ]]></javascript:method>
  </javascript:object>
</javascript:code>