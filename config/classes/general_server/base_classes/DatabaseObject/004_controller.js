<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <!-- this object will:
      initiate the correct JS Class__* linkage based on DatabaseObject class name
      inherit from DatabaseElement to enable basic IDX children [xpath filtered associated object] loading and control
    -->
    <javascript:init><![CDATA[
    ]]></javascript:init>

    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      //dbDatabaseObject
      return (s1Name.length > 1 && s1Name[0] == "d" && s1Name[1] == "b");
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return false;
    ]]></javascript:static-method>

    <!-- ########################################## lazy cached introspection DOMs ########################################## -->
    <javascript:method name="model" parameters="fCallback, [bRecache = false]"><![CDATA[
      //~Person/xsd:schema[not(@name)][*]
      return Database.query(
        XPathString.databaseClass(this).children("xsd:schema").not("@name").hasChildren(),
        {
          optional:true,
          cache:    !bRecache //will add a timestamp to the request
        },
        fCallback
      );
    ]]></javascript:method>

    <javascript:method name="views" parameters="fCallback, [bRecache = false]"><![CDATA[
      //~Person/xsl:stylesheet/xsl:template[str:boolean(@meta:interface-template)]
      return Database.query(
        XPathString.databaseClass(this).children("xsl:stylesheet").children("xsl:template").is("[meta:interface-template]"),
        {
          nodeMask: XPathString.self().or(XPathString.nodeset("xsl:param")),
          optional: true,
          cache:    !bRecache //will add a timestamp to the request
        },
        fCallback
      );
    ]]></javascript:method>

    <javascript:method name="controllers" parameters="fCallback, [bRecache = false]"><![CDATA[
      //~Person/xsl:stylesheet/xsl:template[str:boolean(@meta:javascript-template)]
      return Database.query(
        XPathString.databaseClass(this).children("xsl:stylesheet").children("xsl:template").is("[meta:javascript-template]"),
        {
          nodeMask: XPathString.self().or(XPathString.nodeset("xsl:param")),
          optional: true,
          cache:    !bRecache //will add a timestamp to the request
        },
        fCallback
      );
    ]]></javascript:method>

    <!-- ########################################## lazy cached introspection HTML ########################################## -->
    <javascript:method name="news" NOT_CURRENTLY_USED="true" parameters="fCallback, [bRecache = false]"><![CDATA[
      //fCallback(oHTMLDoc)
      var oHTMLDoc;
      var oParameters = {};

      this.model(function(oXMLDoc){
        HTMLWebpage.mainStylesheet(function(oStylesheet){
          oHTMLDoc = HTMLWebpage.transformWith(oXMLDoc, oStylesheet, fCallback, oParameters, "news");
        });
      }, bRecache);

      return oHTMLDoc;
    ]]></javascript:method>

    <!-- ########################################## this Database ########################################## -->
    <javascript:method name="xJST" parameters="fFunc, oArguments"><![CDATA[
      //executeJavaScriptXSLTemplate()
      //fFunc.tPs = ordered xsl template param names
      //this function oArguments REQUIRES jDisplayObjects first
      //  oArguments: jDisplayObjects REQUIRED, [xsl:params...], [fCallbackSuccess, fCallbackFailure]
      var fCallback, fCallbackSuccess, fCallbackFailure, iNumTempParams, sUnderscoreParamName; 
      var iNumArgs         = oArguments.length;
      var iNumParamArgs    = iNumArgs - 1;      //reduced if there are callbacks
      var oTemplateArgs    = {};
      
      //-------------------------------------------- callback parameters
      //template arguments first: .changename("fart")
      //but we also want:         .changename(Manager)
      //and                       .changename(Manager, function(){...}, function(){...})
      //  (sending a Class through sometimes makes sense as a class node to the template)
      //xsl:params may be optional and have a default
      //however:  ALL callbacks must be a Function, and cannot be classes
      //therefore ALL trailing non-Class functions are callbacks
      if ( iNumArgs > 1 
        && (fCallback = oArguments[iNumArgs-1]) 
        && fCallback instanceof Function
        && !fCallback.definedClass
      ) {
        //at least 1 trailing callback
        fCallbackSuccess = fCallback;
        iNumParamArgs--;
        
        if (iNumArgs > 2
          && (fCallback = oArguments[iNumArgs-2]) 
          && fCallback instanceof Function
          && !fCallback.definedClass
        ) {
          //2 trailing callbacks
          fCallbackFailure = fCallbackSuccess;
          fCallbackSuccess = fCallback;
          iNumParamArgs--;
        }
      }
      
      //-------------------------------------------- param collation
      //the incoming arguments to name value pairs
      if (fFunc.tPs instanceof Array) {
        iNumTempParams = fFunc.tPs.length;
        for (var i = 0; i < iNumTempParams && i < iNumParamArgs; i++) {
          sUnderscoreParamName = fFunc.tPs[i];
          oTemplateArgs[sUnderscoreParamName.underscoreToCamel()] = oArguments[i+1];
        }
      }
      
      return this.updateWithControllerTemplate(fFunc.mode, oTemplateArgs, fCallbackSuccess, fCallbackFailure);
    ]]></javascript:method>

    <javascript:method name="queryWithViewTemplate" parameters="sModeName, oTemplateArgs, [fCallbackSuccess, fCallbackFailure]"><![CDATA[
      return Database.queryObjectWithViewTemplate(this, oTemplateArgs, sModeName, fCallbackSuccess, fCallbackFailure);
    ]]></javascript:method>

    <javascript:method name="updateWithControllerTemplate" parameters="sModeName, [oTemplateArgs, fCallbackSuccess, fCallbackFailure]"><![CDATA[
      Database.updateObjectWithControllerTemplate(this, sModeName, oTemplateArgs, fCallbackSuccess, fCallbackFailure);
    ]]></javascript:method>
  </javascript:object>
</javascript:code>