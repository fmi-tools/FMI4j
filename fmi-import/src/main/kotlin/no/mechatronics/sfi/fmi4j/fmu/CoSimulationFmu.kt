/*
 * The MIT License
 *
 * Copyright 2017-2018 Norwegian University of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING  FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

package no.mechatronics.sfi.fmi4j.fmu

import no.mechatronics.sfi.fmi4j.FmiSimulation
import no.mechatronics.sfi.fmi4j.modeldescription.cs.ICoSimulationModelDescription
import no.mechatronics.sfi.fmi4j.proxy.cs.CoSimulationLibraryWrapper
import no.mechatronics.sfi.fmi4j.proxy.enums.Fmi2Status
import no.mechatronics.sfi.fmi4j.proxy.enums.Fmi2StatusKind
import org.slf4j.Logger
import org.slf4j.LoggerFactory

class CoSimulationFmu internal constructor(
        fmuFile: FmuFile,
        modelDescription: ICoSimulationModelDescription,
        wrapper: CoSimulationLibraryWrapper
) : AbstractFmu<ICoSimulationModelDescription, CoSimulationLibraryWrapper>(fmuFile, modelDescription, wrapper), FmiSimulation {

    private companion object {
        val LOG: Logger = LoggerFactory.getLogger(CoSimulationFmu::class.java)
    }

    override var currentTime: Double = 0.0
        private set

    override fun init(start: Double, stop: Double): Boolean {
        return super.init(start, stop).also {
            currentTime = start
        }

    }

    override fun doStep(dt: Double) : Boolean {

        if (!isInitialized) {
            LOG.warn("Calling doStep without having called init(), " +
                    "remember that you have to call init() again after a call to reset()!")
            return false
        }

        val status = wrapper.doStep(currentTime, dt, true)
        currentTime += dt

        return status == Fmi2Status.OK
    }

    fun cancelStep() = wrapper.cancelStep()

    fun setRealInputDerivatives(vr: IntArray, order: IntArray, value: DoubleArray)
            = wrapper.setRealInputDerivatives(vr, order, value)

    fun getRealOutputDerivatives(vr: IntArray, order: IntArray, value: DoubleArray)
            = wrapper.getRealOutputDerivatives(vr, order, value)

    fun getStatus(s: Fmi2StatusKind) = wrapper.getStatus(s)
    fun getRealStatus(s: Fmi2StatusKind) = wrapper.getRealStatus(s)
    fun getIntegerStatus(s: Fmi2StatusKind) = wrapper.getIntegerStatus(s)
    fun getBooleanStatus(s: Fmi2StatusKind) = wrapper.getBooleanStatus(s)
    fun getStringStatus(s: Fmi2StatusKind) = wrapper.getStringStatus(s)

}