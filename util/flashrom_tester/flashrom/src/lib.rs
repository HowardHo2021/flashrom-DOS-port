//
// Copyright 2019, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Alternatively, this software may be distributed under the terms of the
// GNU General Public License ("GPL") version 2 as published by the Free
// Software Foundation.
//

#[macro_use]
extern crate log;

mod cmd;

use std::{error, fmt};

pub use cmd::{dut_ctrl_toggle_wp, FlashromCmd};

#[derive(Copy, Clone, PartialEq, Debug)]
pub enum FlashChip {
    EC,
    HOST,
    SERVO,
    DEDIPROG,
}

impl FlashChip {
    pub fn from(s: &str) -> Result<FlashChip, &str> {
        let r = match s {
            "ec" => Ok(FlashChip::EC),
            "host" => Ok(FlashChip::HOST),
            "servo" => Ok(FlashChip::SERVO),
            "dediprog" => Ok(FlashChip::DEDIPROG),
            _ => Err("cannot convert str to enum"),
        };
        return r;
    }
    pub fn to(fc: FlashChip) -> &'static str {
        let r = match fc {
            FlashChip::EC => "ec",
            FlashChip::HOST => "host",
            FlashChip::SERVO => "ft2231_spi:type=servo-v2",
            FlashChip::DEDIPROG => "dediprog",
        };
        return r;
    }

    /// Return whether the hardware write protect signal can be controlled.
    ///
    /// Servo and dediprog adapters are assumed to always have hardware write protect
    /// disabled.
    pub fn can_control_hw_wp(&self) -> bool {
        match self {
            FlashChip::HOST | FlashChip::EC => true,
            FlashChip::SERVO | FlashChip::DEDIPROG => false,
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct FlashromError {
    msg: String,
}

impl fmt::Display for FlashromError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.msg)
    }
}

impl error::Error for FlashromError {}

impl<T> From<T> for FlashromError
where
    T: Into<String>,
{
    fn from(msg: T) -> Self {
        FlashromError { msg: msg.into() }
    }
}

pub struct ROMWriteSpecifics<'a> {
    pub layout_file: Option<&'a str>,
    pub write_file: Option<&'a str>,
    pub name_file: Option<&'a str>,
}

pub trait Flashrom {
    /// Returns the size of the flash in bytes.
    fn get_size(&self) -> Result<i64, FlashromError>;

    /// Returns the vendor name and the flash name.
    fn name(&self) -> Result<(String, String), FlashromError>;

    /// Write only a region of the flash.
    fn write_file_with_layout(&self, rws: &ROMWriteSpecifics) -> Result<bool, FlashromError>;

    /// Set write protect status for a range.
    fn wp_range(&self, range: (i64, i64), wp_enable: bool) -> Result<bool, FlashromError>;

    /// Read the write protect regions for the flash.
    fn wp_list(&self) -> Result<String, FlashromError>;

    /// Return true if the flash write protect status matches `en`.
    fn wp_status(&self, en: bool) -> Result<bool, FlashromError>;

    /// Set write protect status.
    fn wp_toggle(&self, en: bool) -> Result<bool, FlashromError>;

    /// Read the whole flash to the file specified by `path`.
    fn read(&self, path: &str) -> Result<(), FlashromError>;

    /// Read only a region of the flash.
    fn read_region(&self, path: &str, region: &str) -> Result<(), FlashromError>;

    /// Write the whole flash to the file specified by `path`.
    fn write(&self, path: &str) -> Result<(), FlashromError>;

    /// Verify the whole flash against the file specified by `path`.
    fn verify(&self, path: &str) -> Result<(), FlashromError>;

    /// Erase the whole flash.
    fn erase(&self) -> Result<(), FlashromError>;

    /// Return true if the hardware write protect of this flash can be controlled.
    fn can_control_hw_wp(&self) -> bool;
}
