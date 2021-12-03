<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <!-- stylesheet cacheing is OFF by default as it should be handled by the Browser cache, not the Javascript 
      see the Response-headers Cache header setting mode="basic_http_headers_client_cache_on"
    -->
    <javascript:static-property name="bEnableClientStylesheetCache">false</javascript:static-property>
    <javascript:static-property name="oCachedStylesheets">new Cache()</javascript:static-property>
    <javascript:static-property name="bRequestInProgress" description="set to true at ajax request start, and to false at end">false</javascript:static-property>
    <javascript:static-property name="bCanHoldMetaData" description="html:html cannot hold meta:information">false</javascript:static-property>

    <javascript:static-capability name="canTransform"><![CDATA[
      return window.ActiveXObject || (document.implementation && document.implementation.createDocument);
    ]]></javascript:static-capability>

    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      return this.displayAdded_html(j1DisplayObject);
    ]]></javascript:method>

    <javascript:method name="displayAdded_html" parameters="j1DisplayObject" chain="true"><![CDATA[
      var self  = this;
      var jBody = j1DisplayObject.find("body:first");

      this.setup(j1DisplayObject); //j(html:html)

      //immediately JS cache our AJAX tree stylesheet
      var sStylesheetHREF = HTMLWebpage.stylesheetHREF();
      if (HTMLWebpage.bEnableClientStylesheetCache && HTMLWebpage.oCachedStylesheets) {
        jBody.addClass("gs-loading-main-stylesheet");
        HTMLWebpage.getStylesheet(sStylesheetHREF, function(hStylesheetDoc) {
          HTMLWebpage.oCachedStylesheets[sStylesheetHREF] = hStylesheetDoc;
          HTMLWebpage.oCachedStylesheets["main"]          = hStylesheetDoc;
          jBody.removeClass("gs-loading-main-stylesheet");
        });
      }
    ]]></javascript:method>

    <javascript:event-handler event="contentchanged" parameters="j1DisplayObject"><![CDATA[
      //this.setup(oOriginalEvent.currentDisplayObject());
      this.setup(j1DisplayObject);
    ]]></javascript:event-handler>

    <javascript:static-method name="clearStylesheetCache"><![CDATA[
      HTMLWebpage.oCachedStylesheets.clear();
    ]]></javascript:static-method>

    <javascript:static-method name="pathname"><![CDATA[
      return document.location.pathname;
    ]]></javascript:static-method>

    <javascript:static-method name="stylesheetHREF"><![CDATA[
      return HTMLWebpage.pathname() + ".xsl";
    ]]></javascript:static-method>

    <javascript:static-method name="mainStylesheet" parameters="fCallback"><![CDATA[
      //TODO: main stylesheet might still be loading...
      if (fCallback instanceof Function) fCallback(HTMLWebpage.oCachedStylesheets["main"]);
      return HTMLWebpage.oCachedStylesheets["main"];
    ]]></javascript:static-method>

    <javascript:static-method name="refresh"><![CDATA[
      document.location = document.location.toString().replace(/#.*/g, '');
    ]]></javascript:static-method>


    <!-- ###################################### setup #################################### -->
    <javascript:method name="setup" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      this.setupBodyInformationClasses(j1DisplayObject); //gs-embedded, gs-js-ok
      this.setupJQueryUIElements(j1DisplayObject);       //gs-jquery-slider, gs-jquery-sortable, gs-draggable, gs-jquery-tabs, gs-jquery-dialog
      //this.setupFunctionLinks(j1DisplayObject);          //a.gs-ajax, a.gs-confirm, gs-backlink
      //this.setupQuickLinks(j1DisplayObject);             //gs-quick-link
      //this.reactToJS(j1DisplayObject);                   //gs-js-hide/show/remove
      //this.beginIFRAMELoadSequencing(j1DisplayObject);   //iframe.gs-delayed-load
    ]]></javascript:method>

    <javascript:method name="setupBodyInformationClasses" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      var jBody = j1DisplayObject.find("body:first");
      if (window != window.top) jBody.addClass("gs-embedded");
      jBody.addClass("gs-js-ok"); //indicate to entire stylesheet that javascript is ok
    ]]></javascript:method>

    <javascript:method name="setupQuickLinks" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      //Quick tips - disable and links and show the first .quicktip instead
      var jBody       = j1DisplayObject.find("body:first");
      var jQuickLinks = j1DisplayObject.find(".gs-quick-link");
      jQuickLinks
        .attr("href", "#")
        .attr("title", "")
        .addClass("quick-linked");
        
      //TODO: move to GS Class for QuickLinks with own event-handlers
        /*
        .click(function() {
          new JOO(this).find(".gs-quick-tip").show("slow");
          Event.currentEvent().stopPropagation(); //quick link
          Event.currentEvent().preventDefault();  //quick link
        })
        .mouseover(function() {
          Event.currentEvent().stopPropagation();
        });
      jBody.on("mouseover", function() { new JOO(".quicktip").hide("slow"); });
      */
    ]]></javascript:method>

    <javascript:method name="reactToJS" parameters="j1DisplayObject"><![CDATA[
      j1DisplayObject.find(".gs-js-remove").remove();
      j1DisplayObject.find(".gs-js-hide").hide();               //hidden by JS
      j1DisplayObject.find(".gs-js-show").show();               //display:none; by default, shown by JS
    ]]></javascript:method>

    <javascript:method name="beginIFRAMELoadSequencing" parameters="j1DisplayObject"><![CDATA[
      //run iframe src after document load so that the browser does not hang at document load
      var jDelayedIFRAMELoadSequencing = j1DisplayObject.find("iframe.gs-delayed-load");
      jDelayedIFRAMELoadSequencing.each(function() {
        if (new JOO(this).attr("id")) new JOO(this).attr("src", new JOO(this).attr("id"));
      });
    ]]></javascript:method>

    <javascript:method name="setupFunctionLinks" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      var self = this;
      var jAJAXLinks = j1DisplayObject.find("a.gs-ajax");

      //TODO: GS event system
      jAJAXLinks.on("click", function(){return self.ajaxLink();});
      j1DisplayObject.find(".gs-backlink").attr("href", "backlink:").on("click", function() { history.go(-1); return false; });
      j1DisplayObject.find("a.gs-check").on("click", function(){
        if (!confirm("are you sure?")) {
          Event.currentEvent().preventDefault();           //a.gs-check => confirm(...)
          Event.currentEvent().stopImmediatePropagation(); //ajax click events etc.
        }
      });
    ]]></javascript:method>

    <javascript:method name="setupJQueryUIElements" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      //standard jquery elements (check for existence first)
      var sComponents = "";

      //sliders
      if (new JOO().slider) {
        sComponents = sComponents.commaAdd("gs-jquery-slider");
        j1DisplayObject.find(".gs-jquery-ui-slider-1").slider({ min: -2, max: 2, steps: 5 });
      }

      //sorting
      if (new JOO().sortable) {
        sComponents = sComponents.commaAdd("gs-jquery-sortable");

        var jSortable = j1DisplayObject.find(".gs-jquery-nodrag");
        var sAxis   = (jSortable.hasClass("gs-sort-axis-x") ? "x" : "y");
        var bRevert = !jSortable.hasClass("gs-sort-norevert");
        jSortable.sortable({ axis: sAxis, opacity: 0.7, revert: bRevert, placeholder: "placeholder", start: HTMLWebpage.sortstart, stop: HTMLWebpage.resorted })
        jSortable.addClass("gs-jquery-candrag")    //apply draggable classes
                 .removeClass("gs-jquery-nodrag"); //remove can't drag classes
        /*
        jSortable.on("sortstop", function(){
          //TODO: re-assign last class...
        });
        */
      }

      //dragging
      if (new JOO().draggable) {
        sComponents = sComponents.commaAdd("gs-draggable");
        //bind new JOO(document) to create, start, drag and stop
        j1DisplayObject.find(".gs-draggable").draggable();
      }

      //dialog
      if (new JOO().dialog) {
        sComponents = sComponents.commaAdd("gs-jquery-dialog");
        j1DisplayObject.find(".gs-jquery-dialog").dialog({ autoOpen: false, modal: true, height: 400, width: 300 });
      }
    ]]></javascript:method>

    <!-- ###################################### workers #################################### -->
    <javascript:method name="ajaxLink" is-protected="true"><![CDATA[
      var sClassname, sFuncname, fFunc, oFormData;
      var jAnchor    = new JOO(Event.currentEvent().target);
      var aClasses   = jAnchor.attr("class").split(' ');
      var i          = 0;
      var sJsonUrl   = jAnchor.attr("href");

      sJsonUrl = sJsonUrl.setUrlParameter("gs__referal", "exclude");

      //find the function to run on return
      for (var i = 0; i < aClasses.length; i++) {
          sClassname = aClasses[i];
          if (sClassname.substr(0,7) == 'ajax_f_') {
              sFuncname = sClassname;
              fFunc     = window[sFuncname];
              if (!fFunc) Debug.error(this, "[" + sFuncname + "] does not exist!"); //required (must always be a response!)
          }
      }

      //run ajax POST
      //normally these links are for server commands
      oFormData = null;
      OO.requireClass("XSchema", "POST ajax link");
      XSchema.post( //we want a POST here so we treat it like a XSchema form
        sJsonUrl,
        oFormData,
        function(oData, sStatus) {
          if (jAnchor) jAnchor.addClass("gs-ajax-submitted");
          if (fFunc)   fFunc(jAnchor, oData, sStatus); //closure
        }
      );

      //prevent normal actions
      Event.currentEvent().preventDefault();  //ajaxLink()
      Event.currentEvent().stopPropagation(); //ajaxLink()
      return false;
    ]]></javascript:method>

    <javascript:method name="highlightMissingLinks" is-protected="true" parameters="j1Selection"><![CDATA[
      j1Selection.find("a").each(function(){
        var self  = this;
        var sHref = new JOO(this).attr("href");
        if (sHref && sHref != '#')
          new JOO(this).get(
            sHref,
            null,
            function(data, status) {HTMLWebpage.highlightMissingLinksReply(oData, sStatus, self);} //closure
          );
      });
    ]]></javascript:method>

    <javascript:method name="highlightMissingLinksReply" is-protected="true" parameters="oData, sStatus, oAnchor"><![CDATA[
      new JOO(oAnchor).addClass("gs-broken-link");
    ]]></javascript:method>

    <javascript:method name="orderDistance" parameters="o1, o2"><![CDATA[
      //TODO: HTMLWebpage::orderDistance() should be in here?
      return o1.distance - o2.distance;
    ]]></javascript:method>

    <javascript:method name="orderTitle" parameters="o1, o2"><![CDATA[
      return o1.title.toLowerCase() > o2.title.toLowerCase();
    ]]></javascript:method>

    <javascript:method name="sortStart" parameters="oUI"><![CDATA[
      oUI.item.parent().parent().addClass("sort-active");
      new JOO(document.body).addClass("sort-inactive");
      return true;
    ]]></javascript:method>

    <javascript:method name="resorted" parameters="oUI"><![CDATA[
      oUI.item.parent().parent().removeClass('sort-active');
      //oUI.item.animate({backgroundColor:'#000000'}, 1000);
      new JOO(document.body).removeClass('sort-inactive');
      return true;
    ]]></javascript:method>

    <!-- ############################################# normalisation ############################################# -->
    <javascript:event-handler event="keypress"><![CDATA[
      //simply disables ctrl-save event for chrome
      if ((Event.currentEvent().which == 115 && (navigator.platform.match("Mac") ? Event.currentEvent().metaKey : Event.currentEvent().ctrlKey)) 
        || (Event.currentEvent().which == 19)) Event.currentEvent().preventDefault();
      return true;
    ]]></javascript:event-handler>

    <javascript:event-handler event="keydown"><![CDATA[
      //used to normalise the cmd+s and ctrl+s events
      var bCTRLKey    = (navigator.platform.match("Mac") ? Event.currentEvent().metaKey : Event.currentEvent().ctrlKey);
      var iLetterCode = Event.currentEvent().which;
      var cLetter     = String.fromCharCode(iLetterCode);

      if (bCTRLKey && iLetterCode > 31) {
        if (Event.currentEvent().shiftKey === false) cLetter = cLetter.toLowerCase();
        bBubble = jQuery.elementWithFocus().trigger("ctrl-" + cLetter);
      }
    ]]></javascript:event-handler>

    <!-- ###################################### load, ajax, transforms ####################################
      all static so that functions can be passed as simple callbacks
      useful for ajax xml -> xsl -> html
      use the "content replacement" below as callbacks to place the result in to the HTMLWebpage
    -->
    <javascript:static-method name="loadDirect" parameters="(String) sHREF, oQueryString, fSuccessCallback, [fFailureCallback, fInitCall]"><![CDATA[
      return this.load(sHREF, oQueryString, fSuccessCallback, fFailureCallback, fInitCall, true);
    ]]></javascript:static-method>

    <javascript:static-method name="load" parameters="(String) sHREF, oQueryString, fSuccessCallback, [fFailureCallback, fInitCall, bLoadDirect = false]"><![CDATA[
      //used by AJAXHTMLLoader to load the HTML content in to a destination
      
      if (fInitCall) fInitCall();
      
      HTMLWebpage.bRequestInProgress = true;
       /* {
        *   //INPUT with data and node-mask
        *   xpath-query-source:id('idx_9488')
        *   xpath-query-database:id('server')
        *   xpath-evaluated-query:*
        *   data:id('groups')
        *   node-mask:descendant-or-self::*
        *   hardlink-policy:suppress-exact-repeat
        *   hardlink-output:follow
        *
        *   //PROCESSING with server transform
        *   interface-mode:children-IDETreeRecord (server-side xsl:template)
        *
        *   //OUTPUT and client transform via @gs:interface-mode
        *   interface-mode:list
        *   explicit-stylesheet:/admin/index.xsl (transform using the interface xsl:template)
        * }
        */
      if (bLoadDirect) sHREF = sHREF.slashPrepend("/direct");

      var oAJAXOptions = {
        url:      sHREF,
        data:     Options.serialiseURLEncoded(oQueryString),
        type:     "GET",
        context:  this, //the this for the callback functions
        cache:    oQueryString.cache,
        success:  function(hData,  sStatus){HTMLWebpage.loadSuccess.call(this, hData,  sStatus, fSuccessCallback);},
        error:    function(oError, sStatus){HTMLWebpage.loadFailure.call(this, oError, sStatus, fFailureCallback);},
      };
      HTTP.ajax(oAJAXOptions);
    ]]></javascript:static-method>

    <javascript:static-method name="loadSuccess" parameters="(Document) hData, sStatus, fCallback"><![CDATA[
      HTMLWebpage.bRequestInProgress = false;
      if (hData.firstElementChild === null) Debug.error(this, "AJAX call returned an empty document");
      fCallback.call(this, hData, sStatus);
    ]]></javascript:static-method>

    <javascript:static-method name="loadFailure" parameters="oError, sStatus, [fCallback]"><![CDATA[
      HTMLWebpage.bRequestInProgress = false;
      if (fCallback) fCallback.call(this, oError, sStatus);
    ]]></javascript:static-method>


    <javascript:static-method name="getStylesheetProcessingHREF" parameters="hXMLDoc"><![CDATA[
      //TODO: compatibility?
      //http://www.w3.org/TR/REC-DOM-Level-1/level-one-core.html
      //<?xml-stylesheet charset="utf-8" type="text/xsl" href="ajaxLoadChildren.xsl"?>
      var sHREF = null;
      var oChildNode;
      var PROCESSING_INSTRUCTION_NODE = 7;
      var rxHREF = / href="([^"]+)"/;

      if ( (hXMLDoc)
        && (hXMLDoc.childNodes)
        && (hXMLDoc.childNodes.length)
      ) {
        //traverse top level nodes looking for the xml-stylesheet PROCESSING_INSTRUCTION_NODE
        for (var i=0; i < hXMLDoc.childNodes.length && !sHREF; i++) {
          oChildNode = hXMLDoc.childNodes.item(i);
          if ((oChildNode.nodeType == PROCESSING_INSTRUCTION_NODE)
          && (oChildNode.target == 'xml-stylesheet' && oChildNode.data)
          && (aMatches = rxHREF.exec(oChildNode.data))
          && (aMatches.length > 1)
          ) {
            sHREF = aMatches[1];
          }
        }
        if (!sHREF) Debug.warn(HTMLWebpage, "failed, no relevant PI found")
      } else Debug.warn(HTMLWebpage, "failed, no Doc or ChildNodes");

      return sHREF;
    ]]></javascript:static-method>

    <javascript:static-method name="runTransform" parameters="hXMLDoc, fTransformCallback"><![CDATA[
      //called "runTransform" to prevent property conflict with database:query "transform" auto-parameter
      //look for stylesheet directive in the XML
      //<?xml-stylesheet charset="utf-8" type="text/xsl" href="ajaxLoadChildren.xsl"?>
      var sHREF = HTMLWebpage.getStylesheetProcessingHREF(hXMLDoc);
      if (sHREF) {
        sHREF = sHREF.substringBefore("?"); //we add our own parameters for this call
        HTMLWebpage.transformWithHREF(hXMLDoc, sHREF, fTransformCallback);
      } else Debug.warn(HTMLWebpage, "no stylesheet associated with the XML");
    ]]></javascript:static-method>

    <javascript:static-method name="transformWithHREF" parameters="hXMLDoc, sHREF, fTransformCallback"><![CDATA[
      var hStylesheetCachedDoc;
      if (HTMLWebpage.bEnableClientStylesheetCache && HTMLWebpage.oCachedStylesheets && (hStylesheetCachedDoc = HTMLWebpage.oCachedStylesheets[sHREF])) {
        //stylesheet XML founc in cache, already processed
        //immediate transform
        HTMLWebpage.transformWith(hXMLDoc, hStylesheetCachedDoc, fTransformCallback);
      } else {
        //AJAX get the stylesheet and then callback to the transformWith(...)
        //defered transform
        HTMLWebpage.getStylesheet(sHREF, function(hStylesheetDoc) {
          HTMLWebpage.oCachedStylesheets[sHREF] = hStylesheetDoc;
          HTMLWebpage.transformWith.call(this, hXMLDoc, hStylesheetDoc, fTransformCallback);
        });
      }
    ]]></javascript:static-method>

    <javascript:static-method name="getStylesheet" parameters="sHREF, fStylesheetCallback, [fStylesheetErrorCallback]"><![CDATA[
      //we always analysis stylesheets for cyclic includes
      //fStylesheetCallback(hStylesheetDoc)
      //var fC = function (hStylesheetDoc, status) {analyseStylesheet(hStylesheetDoc, sHREF, fStylesheetCallback);};
      var oStylesheetOptions = {
        serverSideIncludes:  "yes",  //dump in contents of xsl:includes
        dataType:            "xml"
      };

      if (sHREF.contains("?")) {
        Debug.warn(this, "stylesheet HREF contains parameters already. stripping them [" + sHREF + "]");
        sHREF = sHREF.substringBefore("?");
      }

      var oAJAXOptions = {
        url:      sHREF,
        data:     Options.serialiseURLEncoded(oStylesheetOptions),
        type:     "GET",
        context:  this, //the this for the callback functions
        cache:    true,
        dataType: oStylesheetOptions.dataType,
        success:  fStylesheetCallback,
        error:    fStylesheetErrorCallback
      };
      HTTP.ajax(oAJAXOptions);

      return sHREF;
    ]]></javascript:static-method>

    <javascript:static-method name="namespaceResolver" parameters="sPrefix"><![CDATA[
      var sURI;
      //TODO: drive this namespaceResolver() off the database namespaces list so it is dynamic
      switch (sPrefix) {
        case "gs":     {sURI = GS.NAMESPACE;     break;}
        case "xsl":    {sURI = XSL.NAMESPACE;    break;}
        case "html":   {sURI = HTML.NAMESPACE;   break;}
        case "xhtml":  {sURI = HTML.NAMESPACE;   break;}
        case "xml":    {sURI = XML.NAMESPACE;    break;}
        case "object": {sURI = ObjectClass.NAMESPACE; break;}
        default: {Debug.warn(HTMLWebpage, "namespace prefix [" + sPrefix + "] missing during xpath");}
      }
      return sURI;
    ]]></javascript:static-method>

    <javascript:static-method name="transformWith" parameters="(Node) hXMLDoc, (Document) h1StylesheetDoc, fCallback, [oParameters, sInterfaceMode]"><![CDATA[
      //TODO: client-side xpath query and XSLT abstraction library
      //dynamic content form HTML <object:Thing att="{../xpath-thing}"
      //client XSLT:
      //  http://dev.abiss.gr/sarissa/
      //  https://github.com/ilinsky/jquery-xpath/blob/master/README.md
      var sOldDisplayMode, oRootNode;


      if (window.ActiveXObject) {
        //------------------------------------------------------ IE
        alert('windows not supported yet');
        sHTML = hXMLDoc.transformNode(h1StylesheetDoc);
        //TODO: IE transform oHTMLDoc =
      } else if (document.implementation && document.implementation.createDocument) {
        //------------------------------------------------------ Firefox, Chrome, Safari
        //first child node check
        oRootNode = (hXMLDoc instanceof Document ? hXMLDoc.firstChild : hXMLDoc);
        if (!oRootNode) Debug.error(HTMLWebpage, 'XML document is empty');

        //check there are no xsl:includes
        //Chrome cannot process them
        if (h1StylesheetDoc.documentElement.tagName != 'xsl:stylesheet') Debug.error(HTMLWebpage, 'top level node not xsl:stylesheet');
        if ( h1StylesheetDoc.getElementsByTagNameNS(XSL.NAMESPACE, "include").length
          || h1StylesheetDoc.getElementsByTagNameNS(XSL.NAMESPACE, "import" ).length
        ) {
          Debug.error(HTMLWebpage, 'ajax cannot process client side xsl:includes. try setting @meta:server-side-includes="on"');
        }

        //check all global variables resolve
        //TODO: document.evaluate window.evaluate are not cross-browser. see note above
        var oResults, oAttr, oVar, oVars = h1StylesheetDoc.evaluate('/xsl:stylesheet/xsl:variable', h1StylesheetDoc, HTMLWebpage.namespaceResolver, XPathResult.ANY_TYPE, null);
        oResults = h1StylesheetDoc.evaluate('$test', hXMLDoc, HTMLWebpage.namespaceResolver, XPathResult.ANY_TYPE, null);
        while (oVar = oVars.iterateNext()) {
          if ( (oAttrName   = oVar.attributes['name'])
            && (oAttrSelect = oVar.attributes['select'])
          ) {
            //cannot do this because not in-transform with all variables defined
            //oResults    = h1StylesheetDoc.evaluate('$' + oAttrSelect.value, hXMLDoc, HTMLWebpage.namespaceResolver, XPathResult.ANY_TYPE, null);
          }
        }

        //check that there is no javascript tags in the stylesheet
        //http://stackoverflow.com/questions/435005/xslt-javascript-and-unescaped-html-entities
        //TODO: <html:script warning can check the content as well?
        var oScripts = h1StylesheetDoc.evaluate('//html:script', h1StylesheetDoc, HTMLWebpage.namespaceResolver, XPathResult.ANY_TYPE, null);

        //------------------------------------------------------ optional temporary interface
        if (sInterfaceMode && oRootNode) {
          oRootNode.setAttribute("gs:force-data-interface", sInterfaceMode);
        }

        //------------------------------------------------------ transform
        xsltProcessor = new XSLTProcessor();
        //TODO: transform oParameters
        //xsltProcessor.setParameter(null, "name", "value");
        xsltProcessor.importStylesheet(h1StylesheetDoc);
        //NOTE: transformToDocument() may cause an XML document to be made
        //especially if there is no top level html
        oHTMLDoc = xsltProcessor.transformToFragment(hXMLDoc, document);
        //link up some useful properties
        if (oHTMLDoc) {
          oHTMLDoc.XMLDocument = hXMLDoc;
          oHTMLDoc.XSLDocument = h1StylesheetDoc;
        }
        else Debug.error(HTMLWebpage, "transform failed!");

        if (sInterfaceMode && oRootNode) oRootNode.removeAttribute("gs:force-data-interface");
      }


      if (fCallback) fCallback(oHTMLDoc);
      return oHTMLDoc;
    ]]></javascript:static-method>

    <javascript:static-method NOT_CURRENTLY_USED="true" name="analyseStylesheet" parameters="hStylesheetDoc, fStylesheetCallback, jIncludeReplace"><![CDATA[
      //TODO: client side cyclic xsl:include
      //probably use analyseStylesheet.apply(args) to re-call this same function

      //hStylesheetDoc:  might be a child or the parent
      //sFromHREF:       of this doc
      //fStylesheetCallback(hStylesheetDoc)
      //jIncludeReplace: do we need to do a replacement (indicates child doc)

      var jStylesheetDoc, jIncludes, jInclude, sHREF;
      var jStylesheetParentDoc, vTemplates, jTemplate;
      var these_args       = args;

      jStylesheetChildDoc  = new JOO(hStylesheetDoc);

      //cyclic process includes first
      jIncludes      = jStylesheetDoc.find("[nodeName=xsl:include], [nodeName=xsl:import]");
      if (jIncludes.length) {
        jInclude = jIncludes.first();
        sHREF    = jInclude.attr("href");
        //when finished, callback to here with the same parameters
        HTMLWebpage.getStylesheet(sHREF, function(oStylesheetChildDoc) {
          HTMLWebpage.analyseStylesheet(oStylesheetChildDoc, sFromHREF, fStylesheetCallback, jInclude);
        });
      }

      if (jIncludeReplace) {
        //replace jIncludeReplace with jStylesheetDoc templates. using [nodeName=...] because of the namespaces
        //jIncludeReplace points to the parent stylesheet
        var jTemplates       = jStylesheetDoc.find("[nodeName=xsl:template]");
        var sHREF            = jIncludeReplace.attr("href");

        jIncludeReplace.after(jTemplates);
        jIncludeReplace.remove();
      }

      if (jIncludeReplace) {
        //analyse next xsl:include in the parent
        //now that this one has been replaced
        HTMLWebpage.analyseStylesheet(jIncludeReplace.document, fStylesheetCallback);
        //oCachedStylesheets[sHREF] = oStylesheetMainDoc;
      }
    ]]></javascript:static-method>

    <javascript:static-method name="conditionalTransform" parameters="hDoc, fCallback"><![CDATA[
      if (hDoc instanceof XMLDocument) {
        //we have got an XMLDocument back so we need to client side transform
        //we assume capability to transform was checked at point of sending
        HTMLWebpage.runTransform(hDoc, function(hHTMLDoc) {
          fCallback.call(this, hHTMLDoc);
        });
      } else if (hDoc instanceof HTMLDocument) {
        //HTML data come back so insert it
        fCallback.call(this, hDoc);
      } else {
        Debug.warn(HTMLWebpage, "return type unknown [" + hDoc + "]");
      }
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>