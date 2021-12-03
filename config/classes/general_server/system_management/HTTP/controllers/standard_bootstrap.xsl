<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="standard_bootstrap" version="1.0" extension-element-prefixes="dyn database server">
  <!-- ######################################## templates and variables ######################################## -->
  <xsl:include xpath="process_session"/>             <!-- session processing -->
  <xsl:include xpath="basic_http_headers"/>          <!-- response headers -->
  <xsl:include xpath="client_capabilities"/>         <!-- XSLT capabilities -->
  <xsl:include xpath="map_url_to_stylesheet_model"/> <!-- templating and content output strategy -->
  <xsl:include xpath="utilities"/>                   <!-- HTTP utility templates -->

  <!-- ######################################## standard output ######################################## -->
  <xsl:template match="object:Request/*"/>

  <!-- header output is configured by the stylesheet so we recurse to that first -->
  <!-- xsl:template match="object:Request/gs:HTTP">    <xsl:apply-templates select="." mode="basic_http_headers"/></xsl:template -->
  <!-- xsl:template match="object:Request/gs:cookies"> <xsl:apply-templates select="." mode="process_session"/></xsl:template -->

  <xsl:template match="object:Request/gs:url">
    <xsl:apply-templates select="." mode="map_url_to_stylesheet_model"/>
  </xsl:template>
</xsl:stylesheet>