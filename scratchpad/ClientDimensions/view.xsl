<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" repository:name="view.xsl" repository:type="File" name="view" version="1.0" object:cpp-component="yes">
  <xsl:template match="object:ClientDimensions" mode="editor" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class}">
      ClientDimensions
    </div>
  </xsl:template>
</xsl:stylesheet>