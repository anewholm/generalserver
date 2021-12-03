<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-property name="bRequestInProgress" description="set to true at ajax request start, and to false at end">false</javascript:static-property>
    <javascript:static-property name="bRequestRequiresReport" description="set to true at ajax request start, and to false once reported">false</javascript:static-property>
    <javascript:static-property name="oMonitors">new Object()</javascript:static-property>
    <javascript:static-property name="ajaxTimeoutID"/>
    <!-- lets load the form context inputs as meta data into this object also... -->
    <javascript:static-property name="sAdditionalMetaLocations"><![CDATA["> form > div > .gs-meta-data"]]></javascript:static-property>

    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      /* schema:
       * <div class="Class__XSchema f_submit_ajax ...">
       *   <form class="details">
       *     ...
       *   </form>
       * </div>
       */
      //var schemaName       = jElement.firstClassGroup1(/^gs-xschema-name-([^ "]+)/);
      //var isClientsideOnly = j1DisplayObject.hasClass("gs-clientside-only");
      //this.iAjaxFormError   = 5;
      this.setup(j1DisplayObject);
      //this.focusOnFirstInput(jElement);
    ]]></javascript:method>

    <!-- ######################################## setup ######################################## -->
    <javascript:method name="setup" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      /* schema:
       * <span class="gs-extended-attributes">
       *   <input name="..." value="..." />
       *   [<span class="<type>">...</span>, ...]
       * </span>
       */

      //tested
      //note that all submit() functionality goes through the standard f_submit_ajax
      this.setupAutoSubmits(j1DisplayObject);

      //awaiting testing
      //this.addSubmitButtonToInputSubmits(j1DisplayObject);
      //this.setupDynamicFormActionSwitching(j1DisplayObject);
    ]]></javascript:method>

    <javascript:method name="setupAutoSubmits" is-protected="true" parameters="(JOO) j1Element"><![CDATA[
      var self = this;
      j1Element.find("form").ajaxError(function(){return self.ajaxFormError();});
      j1Element.find(".gs-ajax-autosubmit")
        .change(   function() {return new JOO(this.form).submit();});
      j1Element.find(".gs-ajax-enable-onchange")
        .keyup(    function(){return self.enableOnChange();})
        .change(   function(){return self.enableOnChange();})
        .mouseup(  function(){return self.enableOnChange();});

    ]]></javascript:method>

    <javascript:method name="addSubmitButtonToInputSubmits" is-protected="true" parameters="(JOO) j1Element"><![CDATA[
      j1Element.find("form").submit(function(){
        //forms submitted through the manual call to submit event do not include the submit button
        var sName, sValue;
        var jForm         = new JOO(this);
        var jSubmitButton = jForm.find("input[type='submit']");
        var jHiddenSubmit = jForm.find("input[id='hidden_submit']");
        if (jSubmitButton.length == 1 && jHiddenSubmit.length == 0) {
          sName  = jSubmitButton.attr("name");
          sValue = jSubmitButton.attr("value");
          jSubmitButton.before('<input id="hidden_submit" type="hidden" name="' + sName + '" value="' + sValue + '" />');
          jSubmitButton.removeAttr("name"); //ensure that the submit does not get doubled up

        }
      });
    ]]></javascript:method>

    <javascript:method name="setupDynamicFormActionSwitching" is-protected="true" parameters="(JOO) j1Element"><![CDATA[
      //auto form action switching (ajax only and thus usually only admin screens)
      var jDynamicFormActionSwitching = j1Element.find(":input:has(.gs-extended-attributes .xinput)");
      jDynamicFormActionSwitching.each(function() {
        var jAction = new JOO(this).find("span.action");
        if (jAction.length) {
            input.click(function() {new JOO(this.form).attr("action", sAction.text());});
            input.removeClass("gs-disabled").removeAttr("disabled");
        }
      });

    ]]></javascript:method>

    <!-- ######################################## misc ######################################## -->
    <javascript:method name="focusOnFirstInput" is-protected="true" parameters="jElement"><![CDATA[
      //focus on the first visible textual input that has not asked not to be (comment blocks...)
      //do this after the toggle as it may hide stuff...
      var jFirstInput = jElement.find("input:text, textarea:enabled:not(:hidden):not(.no_initialfocus, .no_initialfocus input:text, .no_initialfocus textarea)");
      if (jFirstInput.length) jFirstInput[0].focus(); //crashes IE if focus not possible
    ]]></javascript:method>

    <javascript:event-handler event="ctrls" parameters="j1DisplayObject"><![CDATA[
      //ctrl-s normalised by the framework Javascript extension
      Event.currentEvent().preventDefault();
      j1DisplayObject.submit();
      return false;
    ]]></javascript:event-handler>

    <!-- ######################################## ajax forms ######################################## -->
    <javascript:event-handler event="submit" parameters="j1DisplayObject, ..."><![CDATA[
      j1DisplayObject.startedSaving();

      if (j1DisplayObject.hasClass("gs-clientside-only")) {
        Event.currentEvent().preventDefault();
      }

      return true; //might be non-ajax
    ]]></javascript:event-handler>

    <javascript:event-handler event="submitfailure" parameters="j1DisplayObject"><![CDATA[
      j1DisplayObject.finishedSaving();
      return true; //might be non-ajax
    ]]></javascript:event-handler>

    <javascript:event-handler event="submitvalidationfailure" parameters="j1DisplayObject"><![CDATA[
      //re-enable all inputs
      //if xsl:stylesheet/@meta:listen-server-events="/object:Server" is on
      //  any data changes in the database will cause a server event and client data to update
      //  so no need to refresh the parent as a result of this addition
      j1DisplayObject.finishedSaving();
      j1DisplayObject.find("input[type='submit']").removeAttr("disabled");
    ]]></javascript:event-handler>

    <javascript:event-handler event="submitsuccess" parameters="j1DisplayObject"><![CDATA[
      //re-enable all inputs
      //if xsl:stylesheet/@meta:listen-server-events="/object:Server" is on
      //  any data changes in the database will cause a server event and client data to update
      //  so no need to refresh the parent as a result of this addition
      j1DisplayObject.finishedSaving();
      j1DisplayObject.find("input[type='submit']").removeAttr("disabled");
    ]]></javascript:event-handler>

    <javascript:event-handler event="submit" action="ajax" parameters="j1DisplayObject, ..."><![CDATA[
      //the server will receive, analyse and process the relevant form configuration
      //simply submit it, along with the identification
      //note that the arguments to this function already contain the form values courtesy of process_f_event
      //  but we re-serialise again
      var jForm         = j1DisplayObject.children("form:first");
      var bSubmit       = true;
      
      if (!jForm.length) {
        Debug.warn(this, "form not found");
        Debug.log(this, j1DisplayObject, true);
      }
      
      //chainMethod
      this.f_submit(j1DisplayObject); 
      
      //inform sub-objects submit is about to happen
      //e.g. CodeMirror sync to textarea
      j1DisplayObject.trigger(PreSubmitEvent); 

      if (jForm.hasClass("gs-check")) bSubmit = confirm("are you sure?");

      if (bSubmit && this.validate(j1DisplayObject)) this.ajaxForm(j1DisplayObject, jForm);

      Event.currentEvent().preventDefault(); //f_submit_ajax()
      return false;
    ]]></javascript:event-handler>

    <javascript:method name="validate" is-protected="true" parameters="j1DisplayObject"><![CDATA[

      var sErrors = '';
      j1DisplayObject.find("form .gs-extended-attributes").each(function() {
          var sRequiredText, rxRegexp, sValueText, sCaptionText, sTypeText, sValidationErrorText;
          var jX                = new JOO(this);
          var jInput            = jX.children(":input");
          var jRequired         = jX.children(".gs-required");
          var jValidationError  = jX.children(".gs-validation-error");
          var sXMessage         = jX.children(".gs-xsd-field-message").text();
          var sXMessageType     = jX.children(".gs-xsd-field-message-type").text();
          var jCaption          = jX.children(".gs-caption");


          //clear hints (default)
          if (sXMessageType == XSDField.fieldMessageType.help && jInput.val() == sXMessage) jInput.val("");

          //validation
          if (jRequired.length) {
              sRequiredText = jRequired.text();
              rxRegexp      = new RegExp(sRequiredText ? sRequiredText : ".+");
              sValueText    = jInput.val();
              sCaptionText  = jCaption.text();
              sTypeText     = jInput.attr("type");
              sValidationErrorText = jValidationError.text();

              if (sValidationErrorText == "") {
                  switch (sRequiredText) {
                      case ".+": {sValidationErrorText = sCaptionText + " is required"; break;}
                      default:   {sValidationErrorText = sCaptionText + " is invalid (" + sRequiredText + ")"; break;}
                  }
              }
              if (sTypeText == "checkbox") sValueText = (jInput.attr("checked") ? sValueText : "off");

              if (!sValueText.match(rxRegexp)) {
                sErrors += "o - " + sValidationErrorText + "\n";
                Debug.warn(this, "[" + jInput.attr("name") + "] failed check [" + sRequiredText + "]");
              }
          }
      });


      if (window.customsubmit) sErrors += customsubmit();

      if (sErrors != "") {
        alert("There are errors in your form:\n" + sErrors);
        Event.currentEvent().stopImmediatePropagation(); //validate() => errors
        Event.currentEvent().preventDefault();           //validate() => errors
      }

      return (sErrors == "");
    ]]></javascript:method>

    <javascript:method name="enableOnChange" is-protected="true"><![CDATA[
      new JOO(this).removeClass("ajax_submitted").find("input[type=submit]").removeClass("gs-disabled").removeAttr("disabled");
    ]]></javascript:method>

    <javascript:method name="ajaxCheckComplete" is-protected="true"><![CDATA[
      //this call may start before the actual ajax request has gone out (!XSchema.bRequestRequiresReport)
      //the calls may miss the actual ajax processing (bRequestInProgress)
      //so: if a call did start and is not in progress
      //then: report it and mark it as reported ready for the next one
      if (XSchema.bRequestRequiresReport && !XSchema.bRequestInProgress) {
        XSchema.bRequestRequiresReport = false; //mark as complete ready for the next ajaxCheckComplete() if any
        return true;
      }
      return false;
    ]]></javascript:method>

    <javascript:method name="ajaxFormError" is-protected="true" parameters="oRequest, oAjaxOptions, thrownError"><![CDATA[
        //oAjaxOptions.url
        //thrownError = HTTP error. This will be undefined if the document failed to parse or the main document was refreshed
        XSchema.bRequestInProgress = false;
        if (iAjaxFormError && thrownError) {
          alert(
            "ajax error (" + iAjaxFormError + ") from [" + (oAjaxOptions ? oAjaxOptions.url : "undefined") + "]:\n" +
            "error:[" + eEvent + "]\n" +
            "request:[" + (oRequest ? oRequest.toString(20) : "undefined") + "]\n" +
            "thrown:[" + thrownError + "]"
          );
          iAjaxFormError--;
        }
    ]]></javascript:method>

    <javascript:method name="ajaxForm" is-protected="true" parameters="j1DisplayObject, jForm"><![CDATA[
      //submit is not included by jQuery in its form serialisation
      //Event.currentEvent().target is the form because that is where the jQuery event was attached
      var sClassname, sFuncname, sResponseFunc, fResponseFunc, sJsonUrl, sFormdata;
      var self       = this;
      var jSubmit    = jForm.find("input[type='submit']");
      var sSubname   = jSubmit.attr("name");
      var sSubval    = jSubmit.attr("value");
      var sAction    = jForm.attr("action");
      var i          = 0;

      //external monitoring with ajaxCheckComplete()
      XSchema.bRequestInProgress     = true;
      XSchema.bRequestRequiresReport = true;

      //we need an action for the ajax POST
      //POST requests will Location: 302 to this URL after the update
      sJsonUrl = (sAction ? sAction : location.pathname);
      //sJsonUrl = sJsonUrl.setUrlParameter("gs__referal", "exclude");

      //look for an optional function to run on return
      if (fResponseFunc = jForm.firstClass(/ajax_f_.*/)) {
        fResponseFunc = this[sResponseFunc];
        if (!Function.istype(fResponseFunc)) Debug.error(this, "[" + sResponseFunc + "] does not exist!"); 
      }

      //serialisation + submit button (not included by jQuery in its form serialisation)
      sFormdata = jForm.serialize();
      sFormdata = sFormdata.ampersandAdd((sSubname ? sSubname : "submit"), (sSubval ? sSubval : "submitted"));

      XSchema.post(
          sJsonUrl, 
          sFormdata,
          this.privateCallback(function(oData, sStatus, jqXHR) {
            XSchema.bRequestInProgress = false;
            if (self.ajaxValid(oData, sStatus)) {

              jForm.trigger("submitsuccess");
              jForm.addClass("gs-ajax-submitted").removeClass("gs-ajax-validation-error").removeClass("gs-error");
              if (fResponseFunc) fResponseFunc(jForm, oData, sStatus); //closure
            } else {
              Debug.warn(this, "triggering [submitvalidationfailure] for ajax form [" + sStatus + "]");
              jForm.trigger("submitvalidationfailure");
              jForm.addClass("gs-ajax-validation-error").addClass("gs-error").removeClass("gs-ajax-submitted");
            }
          }),
          function(jqXHR, sStatus, sErrorThrown) {
            //TODO: cwarn the data for the ajax failure as well
            Debug.warn(this, "triggering [submitfailure] for ajax form [" + sStatus + "] [" + sErrorThrown + "]");
            jForm.trigger("submitfailure");
          }
      );

      //prevent normal actions
      Event.currentEvent().preventDefault();

      return true; //bubble = yes
    ]]></javascript:method>

    <javascript:static-method name="post" is-protected="true" parameters="(String) sJsonUrl, (String) sFormdata, fSuccessCallback, [fFailureCallback]"><![CDATA[
      var oAJAXOptions = {
        type:     "POST",
        url:      sJsonUrl,
        data:     sFormdata,
        context:  this, //the this for the callback functions
        cache:    false,
        dataType: "xml", //TODO: POST MI returns an "ok" Response
        success:  fSuccessCallback,
        error:    fFailureCallback
      };
      HTTP.ajax(oAJAXOptions);
    ]]></javascript:static-method>

    <javascript:method name="ajaxValid" is-protected="true" parameters="hData, sStatus"><![CDATA[
      //convert text to JSON object
      Debug.warn(this, "skipping data validation report anaysis (TODO):\nDATA:\n" + hData + "\nTYPE:" + typeof hData);
      return true;

      //TODO: json data validation stage

      var jsonobj;
      try {jsonobj = eval(oData);}
      catch (e) {
          //JSON object not well formed: hard system error!
          alert('AJAX response badly formed!\n' + e + '\n' + oData);
          return false;
      }
      //JSON object reply well-formed, check details
      var ok = (jsonobj.status == 'ok');
      if (!ok) {
          var validation_errors = "There were validation errors submitting your form\n";
          var error;
          for (i in jsonobj.validation_errors) {
            error = jsonobj.validation_errors[i];
            if (typeof error == 'string') validation_errors += "o - " + jsonobj.validation_errors[i] + "\n";
          }
          alert(validation_errors);
      }
      return ok;
    ]]></javascript:method>

    <!-- ###################################### common ajax reply functions ###################################### -->
    <javascript:method name="ajax_f_debug" is-protected="true" parameters="(JOO) jForm, oData, sStatus"><![CDATA[
      alert('response:\nSTATUS:' + sStatus + '\nDATA:\n' + oData.toString(2));
    ]]></javascript:method>

    <javascript:method name="ajax_f_refresh" parameters="(JOO) jForm"><![CDATA[
      HTMLWebpage.refresh();
    ]]></javascript:method>

    <javascript:method name="ajax_f_removeAncestorLI" is-protected="true" parameters="(JOO) jForm"><![CDATA[
      jForm.parents("li").hide("slow");
    ]]></javascript:method>

    <javascript:method name="ajax_f_removeAncestorTR" is-protected="true" parameters="(JOO) jForm"><![CDATA[
      jForm.parents("tr").hide("slow");
    ]]></javascript:method>

    <javascript:method name="ajax_f_disablesubmit" is-protected="true" parameters="(JOO) jForm"><![CDATA[
      jForm.find("input[type=submit]").attr("disabled", true);
    ]]></javascript:method>
  </javascript:object>
</javascript:code>