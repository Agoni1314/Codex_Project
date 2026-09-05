#pragma once

class Entity
{
public:
    Entity(int id, int gridX, int gridY);

    int GetId() const;
    int GetGridX() const;
    int GetGridY() const;

protected:
    // Entity 只保存逻辑格子坐标，屏幕像素位置由绘制阶段临时计算。
    int id_;
    int gridX_;
    int gridY_;
};
