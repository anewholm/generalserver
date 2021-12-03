<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <!-- ################################### displays ################################### -->
    <javascript:object-property name="jqDisplays">new jQuery()</javascript:object-property>

    <javascript:conversion to-type="JQueryNativeValue"><![CDATA[
      return new JOO(this.jqDisplays);
    ]]></javascript:conversion>

    <javascript:conversion to-type="JOO"><![CDATA[
      return new JOO(this.jqDisplays);
    ]]></javascript:conversion>

    <javascript:method name="displays"><![CDATA[
      return this.toJOO();
    ]]></javascript:method>

    <javascript:method name="displaysWithInterface" parameters="sInterfaceMode"><![CDATA[
      var aDisplaysWithInterface = new Array();
      this.jqDisplays.eachEq(function(){
        if (this.cssInterfaceMode() == sInterfaceMode) aDisplaysWithInterface.push(this.get(0));
      }, DEFAULT);
      return new JOO(aDisplaysWithInterface);
    ]]></javascript:method>

    <javascript:method name="displaysWithDefaultMode"><![CDATA[
      return this.displaysWithInterface("default");
    ]]></javascript:method>

    <javascript:method name="myDisplaysObjectsIn" parameters="(JOO) j1Container, [sPath]"><![CDATA[
      //returns my displayObjects that are IN a given oContainer
      //e.g. this.myDisplaysObjectsIn(jTree)
      var aDisplaysIn = new Array();

      if (sPath) j1Container = j1Container.find(sPath);

      this.jqDisplays.eachEq(function(){
        if (this.closest(j1Container).length) aDisplaysIn.push(this.get(0));
      });

      return new JOO(aDisplaysIn);
    ]]></javascript:method>

    <javascript:method name="hasMyDisplaysIn" parameters="(JOO) j1Container, [sPath]"><![CDATA[
      return (this.myDisplaysObjectsIn(j1Container, sPath).length != 0);
    ]]></javascript:method>

    <javascript:method name="removeInvalidDisplays" is-protected="true"><![CDATA[
      var jq1DisplayObject;
      var iInitialElementCount = this.jqDisplays.length;
      var aInvalidDisplays     = new Array();

      //backwards because we are removing items
      var i = iInitialElementCount;
      while (i) {
        jq1DisplayObject = this.jqDisplays.eq(--i);
        if (!jq1DisplayObject.isInDocument()) {
          this.removeDisplay(jq1DisplayObject);
          aInvalidDisplays.push(jq1DisplayObject);
        }
      }

      //warn
      if (this.jqDisplays.length != iInitialElementCount) {
        Debug.warn(this, "removed displays");
        Debug.log(this, aInvalidDisplays);
      }

      return new JOO(this.jqDisplays);
    ]]></javascript:method>

    <javascript:method name="removeDisplay" is-protected="true" parameters="(jQuery) jq1DisplayObject"><![CDATA[
      this.jqDisplays = this.jqDisplays.not(jq1DisplayObject);
      jq1DisplayObject.remove(); 
    ]]></javascript:method>

    <javascript:method name="hasValidDisplays"><![CDATA[
      return (this.removeInvalidDisplays().length != 0);
    ]]></javascript:method>

    <!-- ################################### misc ################################### -->
    <javascript:method name="privateCallback" is-protected="true" parameters="fMethod, ..."><![CDATA[
      //extended arguments are concatenated on to the *begining* of the arguments sent to the callback
      //e.g. privateCallback(fFunc, uMyArg) => fFunc(uMyArg, [sentArg1, ...])
      var self       = this;
      var aArguments = Array.prototype.slice.call(arguments, 1); 
      var fCallback  = function privateCallback(){
        fMethod.apply(self, aArguments.concat(Array.prototype.slice.call(arguments)));
      };
      fCallback.classDerivedConstructor = this.classDerivedConstructor;
      return fCallback;
    ]]></javascript:method>
    
    <javascript:method name="clone" requires="jQuery"><![CDATA[
      return jQuery.extend({}, this);
    ]]></javascript:method>
      
    <javascript:method name="setTimeout" parameters="fMethod, [iTimeout = 0, ...]"><![CDATA[
      var aArguments = Array.prototype.slice.call(arguments, 0); 
      //remove the time-out
      //in-place array.splice(start, deleteCount)
      Array.prototype.splice.call(aArguments, 1, 1); 
      return window.setTimeout(this.privateCallback.apply(this, aArguments), iTimeout);
    ]]></javascript:method>
  </javascript:object>
</javascript:code>