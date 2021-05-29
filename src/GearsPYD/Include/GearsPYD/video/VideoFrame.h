#pragma once


class VideoFrame
{
    public:
        VideoFrame();
        virtual ~VideoFrame();

        unsigned char*   getYPlane() const;
        unsigned char*   getUPlane() const;
        unsigned char*   getVPlane() const;
        double  getPts() const;
        int     getWidth() const;
        int     getHeight() const;
        int     getYLineSize() const;
        int     getULineSize() const;
        int     getVLineSize() const;

        void storeYPlane(unsigned char* data, int lineSize);
        void storeUPlane(unsigned char* data, int lineSize);
        void storeVPlane(unsigned char* data, int lineSize);
        void setPts(double pts);
        void setWidth(int width);
        void setHeight(int height);

    private:
        unsigned char*   m_YPlane;
        unsigned char*   m_UPlane;
        unsigned char*   m_VPlane;
        double  m_Pts;
        int     m_Width;
        int     m_Height;
        int     m_YLineSize;
        int     m_ULineSize;
        int     m_VLineSize;
};



