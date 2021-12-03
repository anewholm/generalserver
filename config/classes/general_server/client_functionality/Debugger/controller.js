<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:event-handler event="mouseover" action="element" parameters="j1DisplayObject, jTargetElement"><![CDATA[
      jTargetElement.addClass("CSS__Debugger").addClass("gs-debugger-target");

      //xsl:template id
      var sTemplateXMLId  = jTargetElement.attr("debug:template-id");
      var sTemplateClass  = jTargetElement.attr("debug:template-class");
      var sTemplateMatch  = jTargetElement.attr("debug:template-match");
      var sTemplateMode   = jTargetElement.attr("debug:template-mode");
      //object
      var sObjectName     = jTargetElement.attr("debug:object-name");
      var sObjectXMLId    = jTargetElement.attr("debug:object-id");

      var jLI             = new JOO("<li/>");
      if (sTemplateMatch) {
        jLI.text(
            (sTemplateClass ? '<class:' + sTemplateClass  + '>': '')
          + '<xsl:template match="' + sTemplateMatch + '"'
          + (sTemplateMode  ? ' mode="' + sTemplateMode + '"' : '')
          + (sTemplateXMLId ? '' : '')
          + '/>'
          + (sObjectName ?
            ' on <' + sObjectName
            + (sObjectXMLId ? ' ' + sObjectXMLId : '')
            + '/>'
          : '')
          + (sTemplateClass ? '</class:' + sTemplateClass  + '>': '')
        );
        j1DisplayObject.children("ul.gs-template-ids").append(jLI);
      }

      //load extended info...
      /*
      HTMLWebpage.load(
        "~IDE/repository:interfaces/debugger", //=> interface:Debugger
        {
          interfaceMode: "",
          templateId:  new JOO(this).attr("gs:template-id")
        },
        function(){}
      );
      */

      //show interface
      this.jqDisplays.show();

      Event.currentEvent().stopPropagation();
    ]]></javascript:event-handler>

    <javascript:event-handler event="mouseout" action="element" parameters="j1DisplayObject, jTargetElement"><![CDATA[
      jTargetElement.removeClass("CSS__Debugger").removeClass("gs-debugger-target");;
      //j1DisplayObject.children("ul.gs-template-ids").empty();
      //this.jqDisplays.hide();
    ]]></javascript:event-handler>
  </javascript:object>

  <!-- javascript:element_event element=".Object" event="mouseover"><![CDATA[
    if (Event.currentEvent().ctrlKey && Debugger.singleton) Debugger.singleton.f_mouseover_element(new JOO(this));
  ]]></javascript:element_event -->

  <!-- javascript:element_event element=".Object" event="mouseout"><![CDATA[
    if (Debugger.singleton) Debugger.singleton.f_mouseout_element(new JOO(this));
  ]]></javascript:element_event -->
</javascript:code>
