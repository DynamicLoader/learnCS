

// job {0 = return height, 1 = return width}
function judgeDimension(pHeight, pWidth, iHeight, iWidth, job)
{
    var pRatio = pHeight / pWidth;
    var iRatio = iHeight / iWidth;
    var nWidth, nHeight;

    if (pRatio > iRatio) // fit width
    {
        nWidth = pWidth;
        nHeight = pWidth * iRatio
    }
    else // fit height
    {
        nHeight = pHeight
        nWidth = pHeight / iRatio
    }

    if (job === 0)
        return nHeight;
    else if (job === 1)
        return nWidth;
    else return 1;
}
