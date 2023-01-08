package fragments;

import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.application.aiwm.ControlActivityViewModel;
import com.application.aiwm.R;
import com.application.aiwm.joystick.SquareJoystick;

public class MotionFragment extends Fragment {

    ControlActivityViewModel m_viewModel = null;

    public MotionFragment() {
        // Required empty public constructor
    }
    @Override public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.e("MotionFragment", "call onCreateView");
        View root = inflater.inflate(R.layout.fragment_motion, container, false);

        m_viewModel = new ViewModelProvider(getActivity()).get(ControlActivityViewModel.class);

        SquareJoystick joystick = root.findViewById(R.id.squareJoystick);
        joystick.setRangeX(-1000, 1000);
        joystick.setRangeY(-110, 110);
        joystick.m_x.observe(getViewLifecycleOwner(), v -> {
            ((TextView)root.findViewById(R.id.textViewX)).setText(v.toString());
            m_viewModel.swlp.setCurvature(v);
        });
        joystick.m_y.observe(getViewLifecycleOwner(), v -> {
            ((TextView)root.findViewById(R.id.textViewY)).setText(v.toString());
            m_viewModel.swlp.setDistance(v);
        });

        root.findViewById(R.id.buttonMotionUp).setOnClickListener(view -> m_viewModel.swlp.setHeight(-85));
        root.findViewById(R.id.buttonMotionDown).setOnClickListener(view -> m_viewModel.swlp.setHeight(-15));
        return root;
    }

    @Override public void onDestroy() {
        super.onDestroy();
        Log.e("MotionFragment", "call onDestroy");
    }

    @Override public void onDestroyView() {
        super.onDestroyView();
        Log.e("MotionFragment", "call onDestroyView");
    }
}