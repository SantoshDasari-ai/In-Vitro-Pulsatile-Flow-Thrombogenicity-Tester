# Cardiac Pulsatile Flow Simulator

## Overview

This system simulates the pulsatile flow patterns of a human heart using a stepper motor controller. It is specifically designed for laboratory testing of heart valves in controlled fluid loop experiments.

## Features

- Physiologically accurate cardiac cycle simulation
- Distinct systole (contraction) and diastole (relaxation) phases
- Configurable heart rate (default 60 BPM)
- Adjustable stroke volume (default 70ml)
- Precisely timed valve closure at systole/diastole transition
- Asymmetric acceleration/deceleration profiles for realistic flow patterns
- Closed-loop control with real-time parameter adjustment

## Hardware Requirements

- Arduino-compatible microcontroller board
- Stepper motor driver (DM860I or compatible)
- NEMA stepper motor (34HS38-5204S or equivalent)
- Power supply for the stepper driver (24-80V DC)
- Custom fluid loop with heart valve test setup

## Technical Specifications

- Heart rate range: 30-120 BPM (adjustable)
- Default stroke volume: 70ml per cycle (adjustable)
- Tube inner diameter: 25.4mm
- Loop center diameter: 12 inches
- Systole phase: 33% of cardiac cycle
- Diastole phase: 67% of cardiac cycle
- Communication: Serial (115200 baud)

## System Architecture

The control system uses the AccelStepper library to manage precise movement profiles. The cardiac cycle is divided into two primary phases:

1. **Systole (Contraction)**: Rapid clockwise movement with high acceleration
2. **Diastole (Relaxation)**: Gentler counterclockwise return with lower acceleration

Valve closure is synchronized to occur precisely at the transition from systole to diastole.

## Dependencies

- AccelStepper library
- Arduino core libraries
