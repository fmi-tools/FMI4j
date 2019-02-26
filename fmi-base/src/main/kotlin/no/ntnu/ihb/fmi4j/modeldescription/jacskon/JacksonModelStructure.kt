package no.ntnu.ihb.fmi4j.modeldescription.jacskon

import com.fasterxml.jackson.annotation.JsonProperty
import com.fasterxml.jackson.dataformat.xml.annotation.JacksonXmlElementWrapper
import com.fasterxml.jackson.dataformat.xml.annotation.JacksonXmlProperty
import no.ntnu.ihb.fmi4j.modeldescription.ModelStructure
import no.ntnu.ihb.fmi4j.modeldescription.Unknown

/**
 *
 * @author Lars Ivar Hatledal
 */

class ModelStructureImpl: ModelStructure {

    @JacksonXmlProperty(localName = "Outputs")
    private val _outputs: Outputs? = null

    @JacksonXmlProperty(localName = "Derivatives")
    private val _derivatives: Derivatives? = null

    @JacksonXmlProperty(localName = "InitialUnknowns")
    private val _initialUnknowns: InitialUnknowns? = null

    override val outputs: List<Unknown>
        get() = _outputs?.unknowns ?: emptyList()

    override val derivatives: List<Unknown>
        get() = _derivatives?.unknowns ?: emptyList()

    override val initialUnknowns: List<Unknown>
        get() = _initialUnknowns?.unknowns ?: emptyList()

    override fun toString(): String {
        return "ModelStructureImpl(outputs=$outputs, derivatives=$derivatives, initialUnknowns=$initialUnknowns)"
    }

}

/**
 *
 * @author Lars Ivar Hatledal
 */
class Outputs(

        @JacksonXmlProperty(localName = "Unknown")
        @JacksonXmlElementWrapper(useWrapping = false)
        val unknowns: List<JacksonUnknown>? = null

)

/**
 *
 * @author Lars Ivar Hatledal
 */
class Derivatives(

        @JsonProperty("Unknown")
        @JacksonXmlElementWrapper(useWrapping = false)
        val unknowns: List<JacksonUnknown>? = null

)

/**
 *
 * @author Lars Ivar Hatledal
 */
class InitialUnknowns(

        @JsonProperty("Unknown")
        @JacksonXmlElementWrapper(useWrapping = false)
        val unknowns: List<JacksonUnknown>? = null

)
