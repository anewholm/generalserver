<xsl:stylesheet xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:str="http://exslt.org/strings" xmlns:exsl="http://exslt.org/common" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:flow="http://exslt.org/flow" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="constants" version="1.0" extension-element-prefixes="dyn database server str flow debug">
  <!-- ######################################## useful static local variables ######################################## -->
  <xsl:param name="gs_newline" select="'&#10;'"/> <!-- string with entity because non-xpath dependent -->
  <xsl:param name="gs_apos">'</xsl:param>
  <xsl:param name="gs_quote">"</xsl:param>
  <!-- useful for translate(text, $smallcase, $uppercase) -->
  <xsl:param name="gs_lowercase" select="'abcdefghijklmnopqrstuvwxyz'"/>
  <xsl:param name="gs_uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>
</xsl:stylesheet>
