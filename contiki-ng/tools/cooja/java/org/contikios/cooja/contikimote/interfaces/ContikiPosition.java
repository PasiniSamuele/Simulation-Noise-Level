/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

package org.contikios.cooja.contikimote.interfaces;

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;
import java.text.NumberFormat;

import org.contikios.cooja.*;
import org.contikios.cooja.contikimote.ContikiMoteInterface;
import org.contikios.cooja.mote.memory.VarMemory;

import org.contikios.cooja.interfaces.Position;


/**
 * An example of how to implement new mote interfaces.
 *
 * Contiki variables:
 * <ul>
 * <li>char simPositionVar
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>position_interface
 * </ul>
 * <p>
 *
 * This observable never changes.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Position Interface")
public class ContikiPosition extends Position implements ContikiMoteInterface {
  private static Logger logger = Logger.getLogger(ContikiPosition.class);

  private Mote mote = null;
  private VarMemory memory;

  public ContikiPosition(Mote mote) {
    this.mote = mote;
    
    memory = new VarMemory(mote.getMemory());

    Observer observer;
    mote.getInterfaces().getPosition().addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) { 
        mote = (Mote) obj;
        
        Position pos = mote.getInterfaces().getPosition();

        memory.setIntValueOf("coordX", (int) (pos.getXCoordinate() * 100));
        memory.setIntValueOf("coordY", (int) (pos.getYCoordinate() * 100));
        memory.setIntValueOf("coordZ", (int) (pos.getZCoordinate() * 100));

        memory.setByteValueOf("positionChanged", (byte) 1);
      }
    });
    
  }

  public static String[] getCoreInterfaceDependencies() {
    // I need the corresponding C position interface (in position_intf.c)
    return new String[] { "position_interface" };
  }

}
