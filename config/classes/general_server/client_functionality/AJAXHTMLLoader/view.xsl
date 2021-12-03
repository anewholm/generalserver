<xsl:stylesheet xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="interface:AJAXHTMLLoader" mode="editor" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

    <div>
      <xsl:attribute name="class">
        <xsl:value-of select="$gs_html_identifier_class"/>
        <xsl:if test="@func-class-start-notify"> CSS__<xsl:value-of select="@func-class-start-notify"/></xsl:if>
      </xsl:attribute>
      
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </div>
  </xsl:template>
</xsl:stylesheet>