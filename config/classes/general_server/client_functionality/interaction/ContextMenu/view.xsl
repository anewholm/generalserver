<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <!-- TODO: allow these also in the javascript:static-property -->
  <xsl:template match="interface:ContextMenu" mode="gs_extra_classes_overlay_transition">instant</xsl:template>
  <xsl:template match="interface:ContextMenu" mode="gs_extra_classes_overlay_exclusivity">global</xsl:template>
  <xsl:template match="interface:ContextMenu" mode="gs_extra_classes_overlay_position">mouse</xsl:template>
</xsl:stylesheet>