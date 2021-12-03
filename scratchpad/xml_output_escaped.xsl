<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <xsl:strip-space elements="*"/>

  <xsl:template match="*" mode="xml_output_escaped">
      <!-- Begin opening tag -->
      <xsl:text>&lt;</xsl:text>
      <xsl:value-of select="name()"/>

      <!-- Namespaces (not implemented in front end Mozzila browser) -->
      <xsl:for-each select="namespace::*">
          <xsl:text> xmlns</xsl:text>
          <xsl:if test="name() != ''">
              <xsl:text>:</xsl:text>
              <xsl:value-of select="name()"/>
          </xsl:if>
          <xsl:text>="</xsl:text>
          <xsl:apply-templates select="." mode="escape-xml">
              <xsl:with-param name="text" select="."/>
          </xsl:apply-templates>
          <xsl:text>"</xsl:text>
      </xsl:for-each>

      <!-- Attributes -->
      <xsl:for-each select="@*">
          <xsl:text> </xsl:text>
          <xsl:value-of select="name()"/>
          <xsl:text>="</xsl:text>
          <xsl:apply-templates select="." mode="escape-xml">
              <xsl:with-param name="text" select="."/>
          </xsl:apply-templates>
          <xsl:text>"</xsl:text>
      </xsl:for-each>

      <!-- End opening tag -->
      <xsl:text>&gt;</xsl:text>

      <!-- Content (child elements, text nodes, and PIs) -->
      <xsl:apply-templates select="node()" mode="xml_output_escaped"/>

      <!-- Closing tag -->
      <xsl:text>&lt;/</xsl:text>
      <xsl:value-of select="name()"/>
      <xsl:text>&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="text()" mode="xml_output_escaped">
      <xsl:value-of select="."/>
      <!-- xsl:apply-templates select="." mode="escape-xml">
          <xsl:with-param name="text" select="."/>
      </xsl:apply-templates -->
  </xsl:template>

  <xsl:template match="processing-instruction()" mode="xml_output_escaped">
      <xsl:text>&lt;?</xsl:text>
      <xsl:value-of select="name()"/>
      <xsl:text> </xsl:text>
      <xsl:value-of select="."/>
      <!-- xsl:apply-templates select="." mode="escape-xml">
          <xsl:with-param name="text" select="."/>
      </xsl:apply-templates -->
      <xsl:text>?&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="comment()" mode="xml_output_escaped">
      <xsl:text>&lt;!--</xsl:text>
      <xsl:value-of select="."/>
      <!-- xsl:apply-templates select="." mode="escape-xml">
          <xsl:with-param name="text" select="."/>
      </xsl:apply-templates -->
      <xsl:text>--&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="*" mode="escape-xml">
      <xsl:param name="text"/>
      <xsl:if test="$text != ''">
          <xsl:variable name="head" select="substring($text, 1, 1)"/>
          <xsl:variable name="tail" select="substring($text, 2)"/>
          <xsl:choose>
              <xsl:when test="$head = '&amp;'">&amp;amp;</xsl:when>
              <xsl:when test="$head = '&lt;'">&amp;lt;</xsl:when>
              <xsl:when test="$head = '&gt;'">&amp;gt;</xsl:when>
              <xsl:when test="$head = '&quot;'">&amp;quot;</xsl:when>
              <!-- xsl:when test="$head = &quot;&apos;&quot;">&amp;apos;</xsl:when -->
              <xsl:otherwise><xsl:value-of select="$head"/></xsl:otherwise>
          </xsl:choose>
          <xsl:apply-templates select="." mode="escape-xml">
              <xsl:with-param name="text" select="$tail"/>
          </xsl:apply-templates>
      </xsl:if>
  </xsl:template>
</xsl:stylesheet>