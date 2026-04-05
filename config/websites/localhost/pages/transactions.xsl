<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="transactions" meta:handler-for="/config/transactions" version="1.0">
  <database:query data-container-name="transactions" node-mask=".|*" interface-mode="list" data="/config/transactions"/>

  <xsl:template match="gs:root">
    <body id="body">
      <h1>transactions list</h1>
      <ul>
        <xsl:apply-templates select="gs:data/gs:transactions/*" mode="list"/>
      </ul>
    </body>
  </xsl:template>

  <xsl:template match="xmltransaction:*" mode="list">
    <xsl:param name="gs_event_functions"/>
    <li>
      <xsl:if test="@description"><xsl:value-of select="@description"/></xsl:if>
      <xsl:if test="not(@description)"><xsl:value-of select="local-name()"/></xsl:if>
    </li>
  </xsl:template>
</xsl:stylesheet>
