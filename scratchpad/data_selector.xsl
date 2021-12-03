<xsl:stylesheet name="data_selector" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" meta:classes-injection="all" version="1.0">
  <database:query data-name="gs_list_data_selector" display-mode="gs_list_data_selector" database="config" node-mask="*" data="*" />

  <xsl:template match="*" mode="gs_list_data_selector">
    <xsl:apply-templates select="." mode="list">
      <xsl:with-param name="gs_override_display_mode" select="'list_data_selector'"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="gs:root">
    <html xmlns="http://www.w3.org/1999/xhtml">
      <head>
        <title>data selector</title>
        <javascript:code>
          <javascript:global_function name="gs_step_selector_change" parameters="event">
            var jSelect = $(this);
            var jLI     = jSelect.closest("html\\:li,li");
            jLI.removeClass("gs_step_selector_none");
            jLI.removeClass("gs_step_selector_only");
            jLI.removeClass("gs_step_selector_all");
            jLI.removeClass("gs_step_selector_these");
            jLI.addClass('gs_step_selector_' + jSelect.val());
          </javascript:global_function>

          <javascript:raw><![CDATA[
            var jDetailDocument;

            $(document).ready(function(){
              $(".gs_step_selector").change(gs_step_selector_change);

              if (window.parent && window.parent.document) {
                //welcome screen
                jDetailDocument = $(window.parent.document).find("#detail");
                jDetailDocument.attr("src", "/admin/welcome");

                //tree select -> window details
                $(".tree").on("select", function(event, oSelectedObject){
                  jDetailDocument.attr("src", "~IDE/repository:panels/name::multiview?xpath-to-node=" + oSelectedObject.xpathToNode);
                });
              }
            });

          ]]></javascript:raw>
        </javascript:code>

        <css:stylesheet xmlns="http://general_server.org/xmlnamespaces/css/2006" a="a">
          <clause>
            <path><class>gs_step_selector</class></path>
            <font-size metric="pixel">10</font-size>
            <height metric="pixel">16</height>
            <padding metric="pixel">0</padding>
            <display>inline</display>
          </clause>

          <clause>
            <path>
              <class>tree</class>
              <class>gs_step_selector_none</class>
              <class>details</class>
            </path>
            <opacity>0.7</opacity>
            <color type="hash">666666</color>
          </clause>

          <clause>
            <path><class>gs_step_selector_all</class></path>
            <font-weight>bold</font-weight>
          </clause>

          <clause>
            <path><element>body</element></path>
            <border-right>1px solid #d7d7d7</border-right>
            <border-top>1px solid #a7a7a7</border-top>
            <min-height metric="pixel">400</min-height>
            <background-color type="hash">f7f7f7</background-color>
            <text-shadow>2px 2px 5px #ffffff, -2px -2px 5px #ffffff, 2px -2px 5px #ffffff, -2px 2px 5px #ffffff</text-shadow>

            <background-image type="url">/resources/shared/images/general_server/general_server_500_watermark2.png</background-image>
            <background-position>top center</background-position>
          </clause>
        </css:stylesheet>
      </head>

      <body id="body">
        data: <input class="input_full"/>
        <hr/>
        <ul class="tree Class__Server">
          <xsl:apply-templates select="gs:data/gs:list_data_selector/*" mode="gs_list_data_selector"/>
        </ul>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="*" mode="gs_list_class_additions">
    <xsl:text>gs_step_selector_none </xsl:text>
  </xsl:template>

  <xsl:template match="*" mode="gs_list_pre_additions">
    <select class="gs_step_selector">
      <option value="none"/>
      <option value="only">only</option>
      <option value="these">these</option>
      <option value="all">all</option>
    </select>
  </xsl:template>
</xsl:stylesheet>