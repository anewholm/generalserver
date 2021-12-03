<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      j1DisplayObject.o.iTabCounter   = 0;
      j1DisplayObject.o.iActiveTab    = 0;
      j1DisplayObject.o.jLastTab      = null;

      //select the g_sGSEditorTab
      /*
      if (window["g_sGSEditorTab"]) {
        var oActiveTab = new JOO("#tabs > ul").find("#tab-" + g_sGSEditorTab);

        if (oActiveTab.length) {
          j1DisplayObject.o.iActiveTab = oActiveTab.prevAll().length;
          new JOO("#tabs").tabs("option", "active", j1DisplayObject.o.iActiveTab);

        } else Debug.error(this, "setting active tab [" + g_sGSEditorTab + "] not found");
      }
      */

      //jQuery tabs
      if (j1DisplayObject.tabs instanceof Function) {
        this.oTabs = j1DisplayObject.tabs({
          heightStyle: "content",  //this surrounds the content only. so size the content to the required size
          //heightStyle: "fill",   //this calculates wrong, not taking in to account the ui-tabs-nav UL height
          collapsible: true,
          select:function(oUI){
            location.hash = new JOO(oUI.tab).attr("href");
            //update the last accessed page with the href + hash
            //HTTP.ajax({url: "/includes/save_last.php"});
          },
          active:j1DisplayObject.o.iActiveTab
        })
        .addClass("aretabs")        //apply tab ok classes
        .removeClass("notabs");     //remove tab failed classes
      }
    ]]></javascript:method>

    <javascript:event-handler event="resize"><![CDATA[
      this.refresh();
    ]]></javascript:event-handler>

    <javascript:event-handler event="tabsactivate" parameters="j1DisplayObject, oUI"><![CDATA[
      j1DisplayObject.o.jLastTab = oUI.oldTab;
      //notify all sub-objects on this interface that they are now visible
      j1DisplayObject.trigger("appear", [oUI]);
    ]]></javascript:event-handler>

    <javascript:method name="active"><![CDATA[
      //TODO: assuming singleton here...
      var iActive, jLI;
      if (this.oTabs) {
        if (iActive = this.oTabs.tabs("option", "active")) {
          jLI = this.displays().children("ul:first").children().eq(iActive);
        }
      }
      return jLI;
    ]]></javascript:method>

    <javascript:method name="tabFromID" parameters="s1ID"><![CDATA[
      return this.displays().find("> ul:first > li > a[href='#" + s1ID + "']").parent("li");
    ]]></javascript:method>

    <javascript:event-handler event="click" action="tabDelete"><![CDATA[
      this.deleteTab(Event.currentEvent().targetObject(), Event.currentEvent().targetElement().parent());;
    ]]></javascript:event-handler>
    
    <javascript:method name="deleteTab" parameters="j1DisplayObject, j1LI"><![CDATA[
      var sID  = j1LI.children("a").attr("href");
      var jUL  = j1LI.parent();
      var jDIV = jUL.parent().children(sID);

      //check if need to activate another
      if (j1LI == this.active()) {
        if (j1DisplayObject.o.jLastTab.length && j1DisplayObject.o.jLastTab != j1LI) this.activate(j1DisplayObject.o.jLastTab);
        else this.activate(jUL.children().first());
      }

      //delete
      jDIV.remove();
      j1LI.slideLeftHide("fast", function(){j1LI.remove();});
      this.refresh();
    ]]></javascript:method>

    <javascript:method name="setContents" parameters="j1DisplayObject, uContents, [oParams, sTitle]"><![CDATA[
      //public function
      return this.addTab.apply(this, arguments);
    ]]></javascript:method>

    <!-- ########################################### private workers ########################################### -->
    <javascript:method name="addTab" is-protected="true" parameters="j1DisplayObject, uContents, [oParams, sTitle]"><![CDATA[
      //TODO: function overloading!!!!
      if      (String.istype(uContents)) return this.addTabFromTemplate.apply(this, arguments);
      else if (JOO.istype(uContents))    return this.addTabFromJOO.apply(this, arguments);
    ]]></javascript:method>

    <javascript:method name="addTabFromTemplate" is-protected="true" parameters="j1DisplayObject, sTemplateName, [oParams, sTitle]"><![CDATA[
      //tab-templates reside in the interface:SharedInterfaces child .gs-name-sharedinterfaces-tab-templates
      var uRet, jTabTempateDO;
      var jTabTemplates = j1DisplayObject.find(".gs-name-sharedinterfaces-tab-templates:first");

      if (jTabTemplates.length) {
        jTabTempateDO = jTabTemplates.children(CSSPathString.fromClass(sTemplateName));
        if (jTabTempateDO.length) uRet = this.addTabFromJOO(j1DisplayObject, jTabTempateDO, oParams, sTitle);
        else Debug.error(this, "tab-template [" + sTemplateName + "] not found in SharedInterfaces collection");
      } else Debug.error(this, "tab-templates SharedInterfaces collection not found");

      return uRet;
    ]]></javascript:method>

    <javascript:method name="addTabFromJOO" is-protected="true" requires="LinkedServer.singleton" parameters="j1DisplayObject, oSharedContentDisplayObject, [oParams, sTitle]"><![CDATA[
      //layout
      var jTabSelector  = j1DisplayObject.find("> ul:first");
      var jTabNew       = j1DisplayObject.find("> ul:first > #tab-new-tab");

      var sImgCollapse = LinkedServer.singleton.uri.slashAdd("resources/shared/images/collapse.png");
      var sTitleToUse  = (sTitle ? sTitle : "new tab");
      var sID          = "tab-" + j1DisplayObject.o.iTabCounter++;
      var jCloseImage  = this.newIcon("collapse", undefined, {class:"gs-tab-close f_click_tabDelete"});
      var jA           = new JOO("<a/>",   {href:"#" + sID}).text("new tab");
      var jDIV         = new JOO("<div/>", {id:sID});
      var jLI          = new JOO("<li/>").append(jA).append(jCloseImage);
      var jContentDisplayObject;

      //add the tab
      if (jTabNew.length) jTabNew.before(jLI);
      else                jTabSelector.append(jLI);
      j1DisplayObject.append(jDIV);

      //refresh
      this.refresh();
      this.activate(jLI);
      jLI.trigger("tabsadd", [this]); //IDE uses this to remove the welcome tab

      //populate and show it
      //might be AJAX so the f_shown will work out names and things
      jLI.startedLoading();
      jContentDisplayObject = oSharedContentDisplayObject.cloneTo(jDIV, DEFAULT);
      jContentDisplayObject.show(DEFAULT, function(){
        //QualifiedName can be on the first 2 levels
        var sTitle, jQN = jDIV.children(QualifiedName);
        if (!jQN.length) jQN = jDIV.children().children(QualifiedName);
        if (jQN.length) {
          if (sTitle = jQN.contents().get(0).nodeValue) { //so not to include the meta-data
            jLI.children("a").text(sTitle.limit());   //20 + ...
            jLI.children("a").attr("title", sTitle);  //tooltip
          }
        }

        jLI.finishedLoading();
      }, oParams);
    ]]></javascript:method>

    <!-- ########################################### private tab navigation workers ########################################### -->
    <javascript:method name="tabFromJContent" is-protected="true" parameters="jContent"><![CDATA[
      var jPanel = jContent.closest("div.ui-tabs-panel");
      var s1ID   = jPanel.id();
      return (s1ID ? this.tabFromID(s1ID) : undefined);
    ]]></javascript:method>

    <javascript:method name="panelFromID" is-protected="true" parameters="s1ID"><![CDATA[
      return this.displays().children("div[id='#" + s1ID + "']");
    ]]></javascript:method>

    <javascript:method name="refresh" is-protected="true"><![CDATA[
      if (this.oTabs) this.oTabs.tabs("refresh");
    ]]></javascript:method>

    <javascript:method name="activate" is-protected="true" parameters="jLI"><![CDATA[
      //note that we have Class__Debugger items in this UL also...
      var jAllTabs = jLI.parent().children("li");
      var iIndex;
      if (jLI && jAllTabs && jAllTabs.length && jLI.length && this.oTabs) {
        iIndex = jAllTabs.index(jLI);
        this.oTabs.tabs("option", "active", iIndex);
      }
      return iIndex;
    ]]></javascript:method>
  </javascript:object>
</javascript:code>
