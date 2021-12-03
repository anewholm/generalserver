<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" repository:name="view.xsl" repository:type="File" name="view" version="1.0" object:cpp-component="yes">
  <xsl:template match="class:String" mode="split">
    <xsl:param name="primitive"/>
    <xsl:param name="delimiter"/>

    <!-- call this "function" with:
      "string"->split(',')
      <xsl:apply-templates select="class:String" mode="split">
        <xsl:with-param name="primitive" select="[value]" />
        <xsl:with-param name="delimiter" select="[value]" />
      </xsl:apply-templates>
    -->

    <!-- TODO: the function or DELETE this ting -->

  </xsl:template>
</xsl:stylesheet>