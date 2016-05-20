package com.example.linqi.mfdnoisy;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import java.io.File;

public class ImageItemView extends RecyclerView.ViewHolder {

    private ImageView ivPicture;
    //private Button btnDel;
    private String imagePath;
    private Context mContext;
    
    public void setmContext(Context context){
        mContext = context;
    }
    public ImageItemView(View itemView) {
        super(itemView);
        ivPicture = (ImageView) itemView.findViewById(R.id.image_picture);
        ivPicture.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //TODO :Gallery open here
                ComponentName cn = new ComponentName("com.android.gallery3d",
                        "com.android.gallery3d.app.GalleryActivity");
                Intent intent = new Intent();
                intent.setComponent(cn);
                //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intent);
                Log.i("setOnClickListener","button for pictures has been clicked!");
            }
        });

/*        btnDel = (Button) itemView.findViewById(R.id.button_del_picture);
        btnDel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                *//*if(deleteListener != null) {
                    deleteListener.onDelete(imagePath);
                }*//*
                Log.i("setOnClickListener","button for pictures has been clicked!");
            }
        });*/
    }


    public void setImage(String path) {
        File imgFile = new  File(path);
        if(imgFile.exists()){
            imagePath = path;
            Bitmap bitmap = BitmapFactory.decodeFile(imgFile.getAbsolutePath());
            ivPicture.setImageBitmap(bitmap);
        }
    }

    public interface OnDeleteListener {
        void onDelete(String path);
    }
    OnDeleteListener deleteListener;
    public void setOnDeleteListener(OnDeleteListener listener) {
        deleteListener = listener;
    }

}
