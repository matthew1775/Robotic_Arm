// HardwareCommandState.h
#ifndef HARDWARE_COMMAND_STATE_H
#define HARDWARE_COMMAND_STATE_H

struct HardwareCommandState
{
    // --- Główne przeguby ramienia ---
    float joint1_rad, joint2_rad, joint3_rad, joint4_rad;
    float joint5_rad, joint6_rad, joint7_rad, joint8_rad;

    // --- Przeguby pomocnicze MINI ---
    float joint_mini_1_rad, joint_mini_2_rad, joint_mini_3_rad;
    float joint_mini_4_rad, joint_mini_5_rad;

    // --- Prędkości głównych przegubów ---
    float joint1_speed, joint2_speed, joint3_speed, joint4_speed;
    float joint5_speed, joint6_speed, joint7_speed, joint8_speed;

    // --- Prędkości przegubów pomocniczych MINI ---
    float joint_mini_1_speed, joint_mini_2_speed, joint_mini_3_speed;
    float joint_mini_4_speed, joint_mini_5_speed;

    HardwareCommandState() :
        joint1_rad(0.0f), joint2_rad(0.0f), joint3_rad(0.0f), joint4_rad(0.0f),
        joint5_rad(0.0f), joint6_rad(0.0f), joint7_rad(0.0f), joint8_rad(0.0f),
        
        joint_mini_1_rad(0.0f), joint_mini_2_rad(0.0f), joint_mini_3_rad(0.0f),
        joint_mini_4_rad(0.0f), joint_mini_5_rad(0.0f),
        
        joint1_speed(0.0f), joint2_speed(0.0f), joint3_speed(0.0f), joint4_speed(0.0f),
        joint5_speed(0.0f), joint6_speed(0.0f), joint7_speed(0.0f), joint8_speed(0.0f),
        
        joint_mini_1_speed(0.0f), joint_mini_2_speed(0.0f), joint_mini_3_speed(0.0f),
        joint_mini_4_speed(0.0f), joint_mini_5_speed(0.0f)
    {}
};

#endif