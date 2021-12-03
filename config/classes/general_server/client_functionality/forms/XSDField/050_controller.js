<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:enum name="fieldMessageType">
      <javascript:value name="suggestion">"S"</javascript:value>
      <javascript:value name="default">"D"</javascript:value>
      <javascript:value name="help">"H"</javascript:value>
    </javascript:enum>

    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      this.setup(j1DisplayObject);
    ]]></javascript:method>

    <!-- ######################################## setup ######################################## -->
    <javascript:method name="setup" is-protected="true" parameters="j1DisplayObject"><![CDATA[

      //tested
      //note that all submit() functionality goes through the standard f_submit_ajax
      //this.setupLabelHighlights(j1DisplayObject); //TODO: use GS events system
      this.setupExtendedDefaults(j1DisplayObject);
      this.setupJQueryComponents(j1DisplayObject);

      //awaiting testing
      //this.setupAutoCompleteControl(j1DisplayObject);
      //this.setupMonitors(j1DisplayObject); //TODO: use GS events system
      //this.setupAutoFieldSelection(j1DisplayObject);

    ]]></javascript:method>
    
    <javascript:event-handler event="mouseover" selector="label[for]"><![CDATA[
      var jLabel = Event.currentEvent().targetElement();
      var sfor = jLabel.attr("for");
      if (sfor) new JOO("#" + sfor).addClass("gs-labelover");
    ]]></javascript:event-handler>

    <javascript:event-handler event="mouseout" selector="label[for]"><![CDATA[
      var jLabel = Event.currentEvent().targetElement();
      var sfor = jLabel.attr("for");
      if (sfor) new JOO("#" + sfor).removeClass("gs-labelover");
    ]]></javascript:event-handler>

    <javascript:event-handler event="click" selector="label[for]"><![CDATA[
      var jLabel = Event.currentEvent().targetElement();
      var sfor = jLabel.attr("for");
      if (sfor) new JOO("#" + sfor).focus();
    ]]></javascript:event-handler>

    <javascript:method name="setupJQueryComponents" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      var sComponents = "";
      if (jQuery.datepicker) {
        j1DisplayObject.find(".datepicker").datepicker({ dateFormat: 'dd/M/yy' });
        sComponents = sComponents.commaAdd("datepicker");
      }

    ]]></javascript:method>

    <javascript:method name="setupExtendedDefaults" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      //defaults (some disappear on focus etc.)
      var jExtendedDefaults = j1DisplayObject.find(".gs-extended-attributes:has(.gs-xsd-field-message-type)");
      jExtendedDefaults.each(function(){
        var jInput         = new JOO(this).children(":input");
        var sXMessageType  = new JOO(this).children(".gs-xsd-field-message-type").text();
        var sXMessage      = new JOO(this).children(".gs-xsd-field-message").text();

        if (sXMessageType == XSDField.fieldMessageType.help)
          jInput.focus(function() {
            //focus: remove help text, leave default or suggestion
            var jNewinput;
            var jThisInput      = new JOO(this);
            var sThisXMessage   = jThisInput.siblings(".gs-xsd-field-message").text();
            if (jThisInput.val() == sThisXMessage) {
              jThisInput.val("");
              jThisInput.removeClass("gs-input-default");
              if (jThisInput.attr("name") == "form_password") {
                  jNewinput = jThisInput.changeInputType("password");
                  jNewinput.focus();
              }
            }
            return true; //bubble!
          });

        //blur: replace help and default, leave if only a suggestion
        if (sXMessageType == XSDField.fieldMessageType.help || sXMessageType == XSDField.fieldMessageType.default)
          jInput.blur(function() {
            var jThisInput        = new JOO(this);
            var sThisXMessage     = jThisInput.siblings(".gs-xsd-field-message").text();
            var sThisXMessageType = jThisInput.siblings(".gs-xsd-field-message-type").text();
            if (jThisInput.val() == "") {
              jThisInput.val(sThisXMessage);
              if (sThisXMessageType == XSDField.fieldMessageType.help) jThisInput.addClass("gs-input-default");
              if (jThisInput.attr("name") == "form_password") jThisInput.changeInputType("text");
            }
            return true; //bubble up!
          });

        //on startup: set help, default and suggestion
        if (jInput.val() == "" || jInput.val() == sXMessage) {
          jInput.val(sXMessage);
          if (sXMessageType == XSDField.fieldMessageType.help) jInput.addClass("gs-input-default");
          if (jInput.attr("name") == "form_password") jInput.changeInputType("text");
        }
      });

    ]]></javascript:method>

    <javascript:method name="setupAutoFieldSelection" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      //field re-selection on submit (ajax only and thus usually only admin screens)
      //NOTE: requires Javascript
      var jAutoFieldSelection = j1DisplayObject.find(".gs-extended-attributes .xinput");
      jAutoFieldSelection.each(function() {
        var jInput  = new JOO(this).find(":input");
        var jFields = new JOO(this).find("span.fields");
        if (jFields.length) {
          jInput.click(function() {
            new JOO("form *").attr("disabled", true);
            new JOO("form *[name='" + jFields.text() + "']").removeAttr("disabled");
          });
        }
      });

    ]]></javascript:method>

    <javascript:method name="setupAutoCompleteControl" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      var jAutoCompleteControl = j1DisplayObject.find(".noautocomplete");
      jAutoCompleteControl.attr("autocomplete", "off");

    ]]></javascript:method>

    <javascript:method name="setupMonitors" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      //value change monitor - used for typeahead amoungst other things
      var self = this;
      var jMonitors = j1DisplayObject.find(".ajax_monitor");
      jMonitors.each(function(){self.addMonitor();});
      
      //TODO: GS event system...
      new JOO(document).on("click", function() { new JOO(".typeahead ul").hide("slow"); });

    ]]></javascript:method>

    <!-- ######################################## oMonitors ######################################## -->
    <javascript:method name="addMonitor" is-protected="true"><![CDATA[
      //check for the change function
      var self = this;
      var monitor = this;
      var i = 0, func, classes = new JOO(this).attr("class").split(' ');
      while (i < classes.length && classes[i].substr(0,7) != 'ajax_f_') i++;
      if (i < classes.length) {
          this.func = window[classes[i]];
          new JOO(this).find(":input").each(function(){self.addMonitorValue(this, monitor);}); //store intial values
          new JOO(this).keyup(function(){self.monitorChange();}) //typing
              //.change(function(){self.monitorChange();})   //details in 1 input changed
              .mouseup(function(){self.monitorChange();}); //a paste or sumink
      } else alert('no monitor function!');
    ]]></javascript:method>

    <javascript:method name="addMonitorValue" is-protected="true" parameters="(JOO) j1Input, oMonitor"><![CDATA[
      //store the current value (ignoring defaults)
      var inputdefault = j1Input.parent(".gs-extended-attributes.xinput").find("span.gs-xsd-field-message").text();
      var name         = j1Input.attr("name");
      var inputval     = j1Input.val();
      if (inputval == inputdefault) inputval = '';
      XSchema.oMonitors[name]   = inputval;
    ]]></javascript:method>

    <javascript:method name="monitorChange" is-protected="true"><![CDATA[
      var monitor = this;
      new JOO(this).find(":input").each(function(){monitorCompareValue(this, monitor);});
    ]]></javascript:method>

    <javascript:method name="monitorCompareValue" is-protected="true" parameters="(JOO) j1Input, oMonitor"><![CDATA[
      //compare values, this = each :input
      //monitor = the span class="ajax_monitor"
      var inputdefault = j1Input.parent(".gs-extended-attributes.xinput").find("span.gs-xsd-field-message").text();
      var name         = j1Input.attr("name");
      var inputval     = j1Input.val();
      var inputold     = XSchema.oMonitors[name];
      if (inputval == inputdefault) inputval = '';

      if (inputval != inputold) {
          XSchema.oMonitors[name] = inputval;
          //restart timer
          if (XSchema.ajaxTimeoutID) clearTimeout(XSchema.ajaxTimeoutID);
          if (monitor.func) {
              j1Input.addClass('monitor_wait');
              XSchema.ajaxTimeoutID = this.setTimeout(function(){
                  j1Input.removeClass('monitor_wait');
                  monitor.func(monitor, j1Input, inputval);
              }, 1000); //1 second delay
          } else alert('no monitor function!');
      }
    ]]></javascript:method>

    <!-- ################################# typeahead ################################# -->
    <javascript:method name="ajax_f_typeahead" parameters="(JOO) jForm, (JOO) j1Input, sInputVal"><![CDATA[
      //something has changed on a monitored input
      //run a standard ajax function to get the list of typeahead options
      var sHREF, oFormOptions;

      if (sInputVal.length > 2) {
        NOT_COMPLETE(""); //typeahead

        //run ajax
        j1Input.addClass('gs-ajax-loading');
        //TODO: instanciate Options from string? or from Form?
        oFormOptions = jForm.serialize();
        sHREF        = '/api/typeahead/' + j1Input.attr("name") + '.ajaxform?dir:referal=exclude';

        var oAJAXOptions = {
          url:      sHREF,
          data:     Options.serialiseURLEncoded(oFormOptions),
          dataType: "xml",
          type:     "POST",
          context:  this, //the this for the callback functions
          cache:    false,
          success:  function(hData,  sStatus){
                      j1Input.removeClass('gs-ajax-loading').addClass("gs-ajax-loaded");
                      if (ajax_valid(oData, sStatus)) ajax_typeahead(jForm, j1Input, sInputVal, oData, sStatus);
                    },
          error:    function(oError, sStatus){alert("failed!");}
        };
        HTTP.ajax(oAJAXOptions);
      }
    ]]></javascript:method>

    <javascript:method name="ajax_typeahead" is-protected="true" parameters="(JOO) jForm, (JOO) j1Input, sInputVal, oData, sStatus"><![CDATA[
      var option, options = oData.procxml.root.options;
      var typeahead_name  = 'typeahead_' + j1Input.attr("name");
      var typeahead_div   = new JOO('#' + typeahead_name);
      var typeahead_ul;

      //find or create the holding div
      if (typeahead_div.length) {
          //found it
          typeahead_ul = typeahead_div.find("ul");
          typeahead_ul.empty();
      } else {
          //create it
          j1Input.wrap('<div id="' + typeahead_name + '" class="typeahead"></div>');
          j1Input.after('<ul></ul>');
          typeahead_div   = new JOO('#' + typeahead_name);
          typeahead_ul    = typeahead_div.find("ul");
          typeahead_ul.click(function(){typeahead_select(typeahead_ul, jForm, j1Input);});
          typeahead_ul.mouseover(function(){typeahead_mouseover(typeahead_ul, jForm, j1Input);});
          typeahead_div.keydown(function(){typeahead_keydown(typeahead_ul, jForm, j1Input);}); //to act before the form is submitted
      }

      //create options
      var html                        = '';
      var typeahead_custom_optionhtml = window[typeahead_name + '_optionhtml'];
      for (i in options) {
          option = options[i];
          if (typeahead_custom_optionhtml) html += typeahead_custom_optionhtml(option, i);
          else html += '<li>' + option + '</li>';
      }
      if (!html) html = '<li><span class="nooptions">no options found</span></li>';
      typeahead_ul.html(html);
      typeahead_ul.hide();
      typeahead_ul.slideDown("fast");
      j1Input.focus();
    ]]></javascript:method>

    <javascript:method name="typeahead_select" is-protected="true" parameters="typeahead_ul, (JOO) jForm, (JOO) j1Input"><![CDATA[
      var target  = new JOO(Event.currentEvent().target);
      if (target.is("li")) {
          j1Input.val(target.text());
          jForm.submit();
      }
    ]]></javascript:method>

    <javascript:method name="typeahead_mouseover" is-protected="true" parameters="typeahead_ul, (JOO) jForm, (JOO) j1Input"><![CDATA[
      var target  = new JOO(Event.currentEvent().target);
      var selected = typeahead_ul.find("li.selected");
      if (target.is("li")) {
          selected.removeClass("selected");
          target.addClass("selected");
      }
    ]]></javascript:method>

    <javascript:method name="typeahead_keydown" is-protected="true" parameters="typeahead_ul, (JOO) jForm, (JOO) j1Input"><![CDATA[
      var selected = typeahead_ul.find("li.selected");
      selected.removeClass("selected");
      switch (Event.currentEvent().keyCode) {
          case 40: {//KEY_DOWN
              if (!selected.length) selected = typeahead_ul.children(":first");
              else selected = selected.next();
              break;
          }
          case 38: {//KEY_UP
              if (!selected.length) selected = typeahead_ul.children(":last");
              else selected = selected.prev();
              break;
          }
          case 13: {//KEY_ENTER
              if (selected.length) {
                  j1Input.val(selected.text());
                  jForm.submit();
              }
              break;
          }
      }
      selected.addClass("selected");
      return false;
    ]]></javascript:method>
  </javascript:object>
</javascript:code>