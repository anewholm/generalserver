<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:str="http://exslt.org/strings" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" extension-element-prefixes="str" version="1.0">
  <xsl:template match="interface:Debugger">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class} gs-hidden" style="display:none">
      <div class="gs-debugger-title">Debugger</div>
      <xsl:apply-templates mode="gs_view_render"/>
      <ul class="gs-template-ids"/>
    </div>
  </xsl:template>

  <xsl:template match="*" mode="gs_javascript_before_method">
    <!-- avoid circular calls as we will output this.className and stuff
      if (window.Debug) Debug.method(this,"attach","derivedClass, baseClass",arguments);
    -->
    <xsl:param name="gs_object_name" select="'BaseObject'"/>

    <xsl:if test="$gs_request/gs:query-string/gs:debug-javascript-functions = 'on'">
      <xsl:if test="not(@name = 'toString' or @name = 'valueOf' or @name = 'method' or @debug = 'off' or ../@debug = 'off')">
        <xsl:text><![CDATA[    //object.method logging injected by JavaScript class:]]></xsl:text>
        <xsl:value-of select="$gs_newline"/>
        <xsl:text><![CDATA[    if (window.Debug && window.Debug.method) Debug.method(]]></xsl:text>
        <xsl:text>this,</xsl:text>
        <xsl:value-of select="$gs_object_name"/>
        <xsl:text>,"</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>","</xsl:text><xsl:value-of select="@parameters"/><xsl:text>"</xsl:text>
        <xsl:text>,arguments);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="javascript:object" mode="gs_javascript_before_method" response:server-side-only="true">
    <!-- avoid circular calls as we will output this.className and stuff
      if (window.Debug) Debug.method(this,"DatabaseObject","derivedClass, baseClass",arguments);
    -->
    <xsl:param name="gs_object_name" select="'BaseObject'"/>

    <xsl:if test="$gs_request/gs:query/gs:debug-javascript-functions = 'on'">
      <xsl:if test="not(@debug = 'off' or ../@debug = 'off')">
        <xsl:text><![CDATA[    //object.method logging injected by JavaScript class:]]></xsl:text>
        <xsl:value-of select="$gs_newline"/>
        <xsl:text><![CDATA[    if (window.Debug && window.Debug.method) Debug.method(]]></xsl:text>
        <xsl:text>this,</xsl:text>
        <xsl:value-of select="$gs_object_name"/>
        <xsl:text>,"init","",arguments);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
