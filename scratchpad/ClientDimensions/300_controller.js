<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006" repository:name="300_controller.js" repository:type="File" object:cpp-component="yes">
  <javascript:object object-name="inherit" inherits="inherit">
    <javascript:accessor name="height" parameters="[sNewHeight]"><![CDATA[
      //this function only applies to the default mode DOs
      var iHeight;
      this.displaysWithDefaultMode().each(function(){
        if (arguments.length) this.height(sNewHeight);
        iHeight = this.height();
      });
      return iHeight;
    ]]></javascript:accessor>

    <javascript:accessor name="width" parameters="[sNewWidth]"><![CDATA[
      //this function only applies to the default mode DOs
      var iWidth;
      this.displaysWithDefaultMode().each(function(){
        if (arguments.length) this.width(sNewWidth);
        iWidth = this.width();
      });
      return iWidth;
    ]]></javascript:accessor>
  </javascript:object>
</javascript:code>