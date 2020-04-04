package com.samsungnext.widget;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.TextureView;

import com.samsungnext.audiovideoplayersample.Wrnch;

import java.nio.ByteBuffer;

public class PlayerTextureView extends TextureView
	implements TextureView.SurfaceTextureListener, AspectRatioViewInterface {

	private static final String TAG_STATIC = "PlayerTextureView:";
	private final String TAG = TAG_STATIC + getClass().getSimpleName();

	private double mRequestedAspect = -1.0;
	private Surface mSurface;

	public PlayerTextureView(Context context) {
		this(context, null, 0);
	}

	public PlayerTextureView(Context context, AttributeSet attrs) {
		this(context, attrs, 0);
	}

	public PlayerTextureView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		setSurfaceTextureListener(this);
	}

	@Override
	public void onPause() {
	}

	@Override
	public void onResume() {
	}

	/**
	 * set aspect ratio of this view
	 * <code>aspect ratio = width / height</code>.
	 */
	public void setAspectRatio(double aspectRatio) {
		if (aspectRatio < 0) {
			throw new IllegalArgumentException();
		}
		if (mRequestedAspect != aspectRatio) {
			mRequestedAspect = aspectRatio;
			requestLayout();
		}
	}

	/**
	 * measure view size with keeping aspect ratio
	 */
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {

		if (mRequestedAspect > 0) {
			int initialWidth = MeasureSpec.getSize(widthMeasureSpec);
			int initialHeight = MeasureSpec.getSize(heightMeasureSpec);

			final int horizPadding = getPaddingLeft() + getPaddingRight();
			final int vertPadding = getPaddingTop() + getPaddingBottom();
			initialWidth -= horizPadding;
			initialHeight -= vertPadding;

			final double viewAspectRatio = (double)initialWidth / initialHeight;
			final double aspectDiff = mRequestedAspect / viewAspectRatio - 1;

			// stay size if the difference of calculated aspect ratio is small enough from specific value
			if (Math.abs(aspectDiff) > 0.01) {
				if (aspectDiff > 0) {
					// adjust heght from width
					initialHeight = (int) (initialWidth / mRequestedAspect);
				} else {
					// adjust width from height
					initialWidth = (int) (initialHeight * mRequestedAspect);
				}
				initialWidth += horizPadding;
				initialHeight += vertPadding;
				widthMeasureSpec = MeasureSpec.makeMeasureSpec(initialWidth, MeasureSpec.EXACTLY);
				heightMeasureSpec = MeasureSpec.makeMeasureSpec(initialHeight, MeasureSpec.EXACTLY);
			}
		}

		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
	}

	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
		// Create an OpenGL ES 2.0 context

		if (mSurface != null)
			mSurface.release();
		mSurface = new Surface(surface);
	}

	int width = 0;
	int height = 0;

	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
		this.width = width;
		this.height = height;
	}

	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		if (mSurface != null) {
			mSurface.release();
			mSurface = null;
		}
		return true;
	}

	private OverlayView overlayView;

	public void setOverlayView(OverlayView overlayView) {
		this.overlayView = overlayView;
	}

	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture surface) {
		final Bitmap orig = getBitmap();

		final Bitmap bitmap = getBitmap(244, 128);
//		Log.v(TAG, "onSurfaceTextureUpdated: " + bitmap.getHeight());

		int bytes = bitmap.getByteCount();

		ByteBuffer buffer = ByteBuffer.allocate(bytes); // Create a new buffer
		bitmap.copyPixelsToBuffer(buffer); // Move the byte data to the buffer

		byte[] temp = buffer.array(); // Get the underlying array containing the data.
		byte[] pixels = new byte[(temp.length / 4) * 3]; // Allocate for 3 byte BGR

		// Copy pixels into place
		for (int i = 0; i < (temp.length / 4); i++) {
			pixels[i * 3] = temp[i * 4 + 3];     // B
			pixels[i * 3 + 1] = temp[i * 4 + 2]; // G
			pixels[i * 3 + 2] = temp[i * 4 + 1]; // R
		}

		final Point[] points = Wrnch.process(pixels, 244, 128, orig.getWidth(), orig.getHeight());
		overlayView.drawPoints(points);

//		Log.v(TAG, "onSurfaceTextureUpdated done");
	}

	public Surface getSurface() {
		return mSurface;
	}
}
