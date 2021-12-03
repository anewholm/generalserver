<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-property name="cViews"><![CDATA['> .CSS__Menu:first > li']]></javascript:static-property>
    
    <javascript:static-property name="lastView"/>
    <javascript:static-property name="bAutoContainerMaxResize">true</javascript:static-property>

    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      //start on previous selected view
      if (MultiView.lastView) this.viewChange(j1DisplayObject, MultiView.lastView);
    ]]></javascript:method>

    <javascript:event-handler event="click" action="changeView" parameters="j1DisplayObject"><![CDATA[
      var jLI            = Event.currentEvent().firstStatedCSSFunctionElement();
      var sClass         = this.classFromView(jLI);

      this.viewChange(j1DisplayObject, sClass);
    ]]></javascript:event-handler>

    <javascript:method name="viewChange" is-protected="true" parameters="j1DisplayObject, sClass"><![CDATA[
      var self = this;
      var jLI, jContainer;
      var jViews = j1DisplayObject.find(MultiView.cViews); //all LI are potential views

      //view and container
      jLI        = this.viewFromClass(j1DisplayObject, sClass);
      jContainer = this.containerFromClass(j1DisplayObject, sClass);

      if (jLI.length) {
        jViews.removeClass("selected");
        jLI.addClass("selected");
        MultiView.lastView = sClass;

        j1DisplayObject.children(".gs-view-container").hide();
        jContainer.show(DEFAULT, function(){
          //notify all sub-objects on this interface that they are now visible
          j1DisplayObject.trigger(AppearEvent);
          //notify parents
          j1DisplayObject.trigger("viewchange");
        });
      }

      //allow event to bubble for container reactions to interface changes
    ]]></javascript:method>

    <javascript:method name="classFromView" is-protected="true" parameters="j1LI"><![CDATA[
      return j1LI.firstClassGroup1(/^view-(.+)/);
    ]]></javascript:method>

    <javascript:method name="viewFromClass" is-protected="true" parameters="j1DisplayObject, sClass"><![CDATA[
      var jViews = j1DisplayObject.find(MultiView.cViews); //all LI are potential views
      return jViews.filter(".view-" + sClass);
    ]]></javascript:method>

    <javascript:method name="containerFromClass" is-protected="true" parameters="j1DisplayObject, sClass"><![CDATA[
      return j1DisplayObject.children("div.view-" + sClass + "-container");
    ]]></javascript:method>
  </javascript:object>
</javascript:code>