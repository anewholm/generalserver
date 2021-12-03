<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-property name="sDebugFlag">'FORCE_SERVER_XSLT'</javascript:static-property>
    <javascript:static-property name="iErrors" description="number of consequtive errors on AJAX request">0</javascript:static-property>

    <javascript:init><![CDATA[
      //"environment" mode displayAdded_environment() will call listen() at startup
      this.listening = false;
      this.aLog      = new Array();
    ]]></javascript:init>
    
    <javascript:method name="displayAdded_environment" parameters="j1DisplayObject" chain="true"><![CDATA[
      this.listen(j1DisplayObject);
    ]]></javascript:method>

    <javascript:static-method name="debugAJAXOptions" parameters="oOptions"><![CDATA[
      oOptions.url = oOptions.url.setUrlParameter("configuration-flags", Server.sDebugFlag, String.prototype.commaAdd);
    ]]></javascript:static-method>

    <!-- ###################################### listen control ###################################### -->
    <javascript:method name="listen" parameters="[j1DisplayObject, x1XPathListenServerEvents = XPathString.root()]"><![CDATA[
      //called at init by the "environment" mode displayAdded_environment()
      //TODO: new system using the JS interface and a controller template
      var self = this;
      var oRet;
      var oOptions = {};
      
      if (this.listening === true) Debug.warn(this, "already listening");
      this.xpathListenServerEvents = x1XPathListenServerEvents;
      oOptions.xpathToNode       = this.xpathListenServerEvents;
      oRet = this.queryWithViewTemplate(
        "listen-server-events-start",
        oOptions,
        this.privateCallback(this.receiveEvent),
        this.privateCallback(this.receiveError)
      );
      this.listening = true;

      return oRet;
    ]]></javascript:method>

    <javascript:method name="stop"><![CDATA[
      this.listening = false;
    ]]></javascript:method>

    <javascript:method name="receiveError" is-protected="true" parameters="..."><![CDATA[
      var iWaitSeconds;
      this.listening = false;
      Server.iErrors++;
      iWaitSeconds = (Server.iErrors > 10 ? 10 : Server.iErrors);
      
      Debug.warn(this, "receiveError on listen events"); //only warn because happens during page shutdown
      this.setTimeout(this.listen, iWaitSeconds * 1000);
    ]]></javascript:method>

    <javascript:method name="receiveEvent" is-protected="true" parameters="jqDataChildren, hDocument, sStatus, ..."><![CDATA[
      var oRet;
      this.listening = false;
      Server.iErrors = 0;
      
      //process
      oRet = this.processEvent(jqDataChildren);
      //use a timeout just in case the server is responding immediately but without error
      this.setTimeout(this.listen, 500);
      
      return oRet;
    ]]></javascript:method>

    <javascript:method name="processEvent" is-protected="true" parameters="jqDataChildren"><![CDATA[
      /* jqDataChildren start point is the <gs:data>/* node
       * 
       * gs:primary-path is an ancestor path from the affected node
       * some of these nodes may be available on the client
       * the first available descendant should handle the event
       * <object:Server> is usually at the top and will handle the event if on the client
       * 
       * <object:Response>
       *   ...
       *   <gs:data>
       *     <gs:server-event>  <====== standard jqDataChildren start nodes
       *       <gs:node-changed>
       *         <gs:primary-path> REQUIRED
       *           primary node    REQUIRED => ServerNodeChangedEvent
       *           [... ancestors ...]      => ServerDescendantNodeChangedEvent
       *           object:Server            => ServerDescendantNodeChangedEvent
       *         </gs:primary-path>
       *       </gs:node-changed>
       *     </gs:server-event>
       *     [...]
       *   </gs:data>
       * </object:Response>
       */
      var self = this;
      var oRet = true;

      jqDataChildren.filter("gs\\:server-event").each(function(){             //<gs:server-event>
        var jqServerEventType = new jQuery(this).children();                  //<gs:node-changed>*
        jqServerEventType.each(function(){
          //-------------- <gs:node-changed> => new ServerNodeChangedEvent(<gs:node-changed>)
          var fEventClass, sCurrentXMLId, oObject;
          var jqEventTypeNode  = new jQuery(this);                            //<gs:node-changed>
          var sTagName         = jqEventTypeNode.tagName().removeNamespace(); //node-changed
          var sClassTypeName   = sTagName.dashToCamel().capitalise();         //NodeChanged
          var sClassNameStub   = sClassTypeName + "Event";                    //NodeChangedEvent
          var sClassName       = "Server" + sClassNameStub;                   //ServerNodeChangedEvent
          var sDescendantClassName = "ServerDescendant" + sClassNameStub;     //ServerDescendantNodeChangedEvent
          var fClass           = window[sClassName] || window.ServerEvent;    //ServerNodeChangedEvent()
          var fDescendantClass = window[sDescendantClassName] || window.ServerEvent; //ServerDescendantNodeChangedEvent()
        
          //-------------- <gs:primary-path> => first primary node on which to trigger the event
          var jqPrimaryPath    = jqEventTypeNode.children("gs\\:primary-path");
          var jqPrimaryNodes   = jqPrimaryPath.children();
          
          if (!jqPrimaryNodes.length) Debug.error(this, "no server event primary-path");
          else {
            //-------------- find the first available client object in list
            fEventClass = fClass;
            jqPrimaryNodes.eachEq(function(){
              var bStop;
              if ( (sCurrentXMLId  = this.xmlXPathToNode())
                && (oObject = OO.fromIDX(sCurrentXMLId))
                && (oObject.trigger instanceof Function)
              ) {
                //send through the primary info
                //so the event handler can ascertain what happened also
                oObject.trigger(fEventClass, [jqPrimaryNodes]);
              }

              //move to the descendant event for the remaining primary-path
              fEventClass = fDescendantClass;
              //stop when first one is found
              bStop  = (oObject !== undefined)
              return !bStop; //false causes stop
            });
          }

          //logging
          //avoid recording the actual DOM elements so that they are garbage collected
          self.aLog.push([sTagName, fEventClass, sCurrentXMLId, oObject]);
        });
      });

      return oRet;
    ]]></javascript:method>
  </javascript:object>
</javascript:code>