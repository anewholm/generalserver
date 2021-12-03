<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      j1DisplayObject.o.loaded = false;

      //checks
      if (!this.xpathToNode)        Debug.warn(this, "xpathToNode missing for AJAXHTMLLoader. element will not be located in query");
      if (!this.data && !this.dataID) Debug.warn(this, "data[ID] missing for AJAXHTMLLoader. no data can be selected");
    ]]></javascript:method>
    
    <javascript:method name="toggle" parameters="j1DisplayObject, [oOptions = {}, fCallback = Debug.callback]"><![CDATA[
      return this.show.apply(this, arguments);
    ]]></javascript:method>

    <javascript:method name="show" parameters="j1DisplayObject, [oOptions = {}, fCallback = Debug.callback, oParams]"><![CDATA[
      //run the intended show() after the load
      //event chaining is also possible but that would re-run everything else that has happened also
      //  e.g. addTab(...)
      this.load(j1DisplayObject, function(jNewElements){
        jNewElements.show(oOptions, fCallback);
      }, oParams);
    ]]></javascript:method>

    <javascript:method name="load" parameters="j1DisplayObject, fCallback = Debug.callback, [oDatabaseQuery = new Object(), oContext]"><![CDATA[
      //fCallback() => pointing to the original show()
      //load from the id of the <database thing
      var self             = this;
      var oDisplayData, sExpando, j1Object;
      var sHREF            = this.xpathToNodeClient.toAbsoluteURL();     //@xml:id of the original database:query
      var jContainerDO     = j1DisplayObject.parent().closest(Object);
      var eOriginalEvent   = Event.currentEvent();
      var bParameterisedDataQuery = (this.data && this.data.containsParameter());
      var jTargets, jNewElements;
      var bOnceOnly, bLoadDirect, bChainEvent, fInsert, fClassNotify;

      //--------------------------------------- This Object info
      //e.g. a shared Class__AJAXHTMLLoader accessed by the Controller
      //we also send through some more required possible dynamically generated attributes:
      oDatabaseQuery.dataContext            = j1DisplayObject.optional("data-context",    oContext && oContext.xpathToNode.toAbsoluteURL());
      oDatabaseQuery.xpathToContainerNodeDB = jContainerDO && jContainerDO.toXPathString().toAbsoluteURL();
      oDatabaseQuery.xpathToAncestorNodeDBs = j1DisplayObject.ancestors(Object).toXPathString().toAbsoluteURL();

      //--------------------------------------- Event Objects info
      //we append -target instead of prepend because we do not want to confuse variable types
      //include display specific data like interface-mode
      //Objects can override this behaviour by implementing data(j1DisplayObject)
      //
      //e.g. the Class__SubMenu holding the <LI>
      if (j1Object = eOriginalEvent.targetObject()) {
        if (oDisplayData = j1Object.data())
          for (sExpando in oDisplayData)
            oDatabaseQuery[sExpando + "Target"] = oDisplayData[sExpando];
        oDatabaseQuery.xpathToTargetNodeDB          = j1Object.toXPathString().toAbsoluteURL();
        oDatabaseQuery.xpathToTargetAncestorNodeDBs = j1Object.ancestors(Object).toXPathString().toAbsoluteURL();
        oDatabaseQuery.xpathToTargetChildrenNodeDBs = j1Object.children( Object).toXPathString().toAbsoluteURL();
      }

      //e.g. the Class__Group or Class__IDE holding the f_event_function
      if (j1Object = eOriginalEvent.currentDisplayObject()) {
        if (oDisplayData = j1Object.data())
          for (sExpando in oDisplayData) 
            oDatabaseQuery[sExpando + "Current"] = oDisplayData[sExpando];
        oDatabaseQuery.xpathToCurrentNodeDB          = j1Object.toXPathString().toAbsoluteURL();
        oDatabaseQuery.xpathToCurrentAncestorNodeDBs = j1Object.ancestors(Object).toXPathString().toAbsoluteURL();
        oDatabaseQuery.xpathToCurrentChildrenNodeDBs = j1Object.children(Object).toXPathString().toAbsoluteURL();
      }

      //--------------------------------------- these settings
      //if not specified then defaults to:
      //  database:query will be replaceNode() by the new content
      //  chain event on to new objects (false), e.g. the contextmenu will receive the rightclick event also
      bOnceOnly     = j1DisplayObject.setting("is-once-only",   true);        //default: remove display after
      bLoadDirect   = j1DisplayObject.setting("is-load-direct", false);       //no interpretation of interface-render
      fClassNotify  = j1DisplayObject.optional("func-class-start-notify");    //e.g. loading placeholders
      fInsert       = j1DisplayObject.setting("func-insert", JOO.prototype.mergeNodes);
      jTargets      = j1DisplayObject.setting("dom-targets", j1DisplayObject); //can use "parent" here
      bChainEvent   = j1DisplayObject.setting("is-chain-event", false);

      //--------------------------------------- sanity checks
      //auto re-triggering the event after load
      //so that the loaded objects events get a normal chance to react to the event
      if (bChainEvent && !bOnceOnly) Debug.error(this, "request to run the event twice but without removing the original AJAXHTMLLoader");
      if (oContext && (oContext.xpathToNode === undefined) && (oContext.xmlID === undefined))
        Debug.warn(this, "direct context object sent through with no identification on it");
      if (this.jqDisplays.length > 1 && !bParameterisedDataQuery && (oDatabaseQuery.dataContext === undefined) && (oDatabaseQuery.dataContextID === undefined))
        Debug.warn(this, "strange: AJAXHTMLLoader has multiple displayObjects but no @data-context[-id], thus context will be fixed by the @data only");
      if (!jTargets || jTargets.length == 0)
        Debug.error(this, "AJAXHTMLLoader has no dom-targets to populate");

      //--------------------------------------- load
      HTMLWebpage.load(
        sHREF, //xpath to the database:query such that it renders itself
        oDatabaseQuery,
        function(oXMLDoc, sStatus){
          HTMLWebpage.conditionalTransform(oXMLDoc, function(oHTMLDoc){
            var jqHTMLDoc = new jQuery(oHTMLDoc);
            //e.g. JOO.prototype.mergeNodes(...)
            if (oHTMLDoc.firstElementChild === null) Debug.warn(this, "AJAX load transform returned an empty doc");
            //fInsert will trigger "contentchanged" event:
            //and load the objects and displayAdded_default(...)
            //merge(jqTargets,jqSources,fFirstIndexOf,bRemoveOldTargets,uLimitSelector,jqStartTarget)
            jNewElements = fInsert.call(jTargets, jqHTMLDoc.children());
            
            //the user has made this AJAX query on demand
            //meaning the content is not visible on first request
            //the new nodes will display immediately in their DOM insert
            //but the user is expecting them to be hidden: so we hide them
            //toggle calls will then show them
            jNewElements.hideDirect(); 
            
            //the fInsert may have removed the <database:query> element, making this display invalid
            //or the settings (bOnceOnly) may require its removal
            //so: also disconnect events, so that the subsequent chain-event will not re-trigger the same action
            j1DisplayObject.o.loaded = true;
            if (bOnceOnly) {
              //normal default:
              //@once-only requests that the database:query is removed whatever happens
              //so event re-triggering can happen to trigger any new attached events from the new objects
              self.removeDisplay(j1DisplayObject);
              if (bChainEvent) eOriginalEvent.retrigger();
            } else if (j1DisplayObject.isInDocument()) {
              //@once-only=false and the database:query is still there
              //DO NOT re-trigger the event because it will infinite loop re-trigger THIS ODDQ
              //it is the default NOT to chain the event if the ODDQ has not been removed
              if (bChainEvent) Debug.warn(self, "refusing to event.retrigger() if the original database:query was not removed");
            } else {
              //@once-only has EXPLICITLY been set to false
              //thus no required removal BUT the <database:query> HAS been removed by the fInsert:
              Debug.warn(self, "@once-only=false but the database:query was removed by the fInsert already");
              self.removeDisplay(j1DisplayObject);
              //it is the default to chain the event if the ODDQ display has been removed
              //e.g. run the rightclick on the new contetmenu
              //the event.target may have disappeared, but retrigger will check that
              //removeDisplay above will have unhooked the ODDQ/Overlay events
              //and "contentchanged" will have attached any new events to the same place
              if (bChainEvent) eOriginalEvent.retrigger();
            }
            
            //callback last when our AJAX DO is removed
            Event.setCurrentEvent(eOriginalEvent);
            if (fCallback) fCallback(jNewElements);
            Event.clearCurrentEvent();

            //loading complete notices
            if (fClassNotify instanceof Function) fClassNotify.finishedLoading(j1DisplayObject);
            else if (jContainerDO) jContainerDO.finishedLoading();
          });
        },
        function(oError, sStatus) {Debug.error(self, "oops:\n" + sHREF + "\n" + sStatus);},
        function(){
          //loading started notices
          if (fClassNotify instanceof Function) fClassNotify.startedLoading(j1DisplayObject);
          else if (jContainerDO) jContainerDO.startedLoading();
        },
        bLoadDirect
      );
    ]]></javascript:method>
  </javascript:object>
</javascript:code>
