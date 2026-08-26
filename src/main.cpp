        #include "raylib.h"
        #include "raymath.h"
        #include <vector>



        enum class State {Idle, Walking, Mimicking};
        struct Agent
        {
            Vector2 AgentPosition;
            Color AgentColor;
            Vector2 TargetPosition;
            State AgentState;
            float TimeStanding;
            float PauseDuration;
            bool IsMimic;
        };

        int main()
        {
            const int SCREEN_WIDTH = 960;
            const int SCREEN_HEIGHT = 540;
            std::vector<Vector2> PlayerRecordedDeltaPositions;
            Vector2 PlayerStartingPosition;
            Vector2 PlayerDeltaPosition;
            bool hasHistory;
            Vector2 delayedData;
            bool delayedMoving;
            bool clickedThisAgent;
            
            InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CopyCat");
            std::vector<Agent> Agents{};
            Color Palette[] = {PURPLE,BLUE,GREEN,YELLOW,ORANGE,BROWN};
            for(Color& color : Palette)
            {
                Agent NewAgent =
                {
                    {(float)GetRandomValue(0, SCREEN_WIDTH), (float)GetRandomValue(0, SCREEN_HEIGHT)},
                    color,
                    Vector2Clamp(Vector2{(float)GetRandomValue(0, SCREEN_WIDTH), (float)GetRandomValue(0, SCREEN_HEIGHT)}, Vector2{0.0f,0.0f}, Vector2{SCREEN_WIDTH, SCREEN_HEIGHT}),
                    State::Walking,
                    0.0f,
                    0.0f,
                    false
                };
                Agents.push_back(NewAgent);
            }

            Agents[GetRandomValue(0, 5)].IsMimic = true;

            SetTargetFPS(60);
            Vector2 position = Vector2{480.0f,270.0f};
            Vector2 direction = Vector2{0.0f,0.0f};
            float speed = 200.0f;

            while(!WindowShouldClose())
            {
                PlayerStartingPosition = position;
                direction = Vector2{0.0,0.0};
                
                if(IsKeyDown(KEY_UP))
                {
                    direction.y += -1;
                }
                if(IsKeyDown(KEY_DOWN))
                {
                    direction.y += 1;
                }
                if(IsKeyDown(KEY_RIGHT))
                {
                    direction.x += 1;
                }
                if(IsKeyDown(KEY_LEFT))
                {
                    direction.x += -1;
                }
                direction = Vector2Normalize(direction);
                position = Vector2Add(position, Vector2Scale(direction, speed*GetFrameTime()));

                PlayerDeltaPosition = Vector2Subtract(position, PlayerStartingPosition);
                PlayerRecordedDeltaPositions.push_back(PlayerDeltaPosition);
                hasHistory = PlayerRecordedDeltaPositions.size() >= 90;
                delayedData = hasHistory ? PlayerRecordedDeltaPositions[PlayerRecordedDeltaPositions.size() - 90] : Vector2 {0,0};
                delayedMoving = Vector2Length(delayedData) > 0.5f;

                for (Agent& agent: Agents) 
                    {
                        switch (agent.AgentState) 
                        {
                            case State::Idle:
                            {
                                agent.TimeStanding += GetFrameTime();
                                if(agent.TimeStanding >= agent.PauseDuration)
                                {
                                    agent.AgentState = State::Walking;
                                }
                                if(agent.IsMimic && delayedMoving)
                                {
                                    agent.AgentState = State::Mimicking;
                                }
                                break;
                            }
                            case State::Walking:
                            {
                                Vector2 WalkingDirection = Vector2Normalize(Vector2Subtract(agent.TargetPosition, agent.AgentPosition));
                                agent.AgentPosition += Vector2Scale(WalkingDirection, GetFrameTime() * 130);
                                if(Vector2Distance(agent.AgentPosition, agent.TargetPosition)<= 5.0f)
                                {
                                    agent.TimeStanding = 0.0f;
                                    agent.TargetPosition = Vector2Clamp(agent.AgentPosition + Vector2{(float)GetRandomValue(-450, 450), (float)GetRandomValue(-250, 250)}, Vector2{0.0f,0.0f}, Vector2{SCREEN_WIDTH, SCREEN_HEIGHT});
                                    agent.AgentState = State::Idle;
                                    agent.PauseDuration = (float)GetRandomValue(6, 12)/10;
                                }
                                if(agent.IsMimic && delayedMoving)
                                {
                                    agent.AgentState = State::Mimicking;
                                }
                                break;
                            }
                            case State::Mimicking:
                            {
                                agent.AgentPosition += delayedData;
                                if(!delayedMoving)
                                {
                                    agent.TimeStanding = 0.0f;
                                    agent.AgentState = State::Idle;
                                }
                                break;
                            }
                        }
                    }

                BeginDrawing();
                    ClearBackground(LIGHTGRAY);
                    for (Agent& agent: Agents) 
                    {
                        DrawCircleV(agent.AgentPosition, 20.0f, agent.AgentColor);
                        if(IsKeyDown(KEY_M) && agent.IsMimic== true)
                        {
                            DrawCircleV(agent.AgentPosition, 10.0, WHITE);
                        }
                        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                        {
                            clickedThisAgent = CheckCollisionPointCircle(GetMousePosition(), agent.AgentPosition, 25.0f);
                            if(clickedThisAgent)
                            {
                                if(agent.IsMimic)
                                {
                                    DrawText("You Won!!", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 48, GREEN);
                                }
                                else 
                                {
                                    DrawText("You Lose!", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 48, RED);
                                }
                            }
                        }
                    }
                    DrawCircleV(position, 20.0f, RED);
                    DrawText("Trust No One", 0, 0, 48, BLACK);
                EndDrawing();
            }

            CloseWindow();
                
            return 0;
        }
